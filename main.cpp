#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

#include "rbtree.hpp"
#include "stack.hpp"

struct PietStackNaive {
 public:
  PietStackNaive() : data() {}
  void push(int32_t val) { data.push_back(val); }
  int32_t pop() {
    if (data.empty()) throw std::out_of_range("the stack is empty");
    int32_t val = data.back();
    data.pop_back();
    return val;
  }
  void roll(int32_t depth, int32_t count) {
    if (depth < 0 || static_cast<size_t>(depth) > std::size(data))
      throw std::out_of_range("Invalid depth");
    if (depth == 0) return;  // nothing to do
    count %= depth;
    if (count < 0) count += depth;  // make positive
    std::rotate(std::end(data) - depth, std::end(data) - count, std::end(data));
  }
  size_t size() const { return std::size(data); }
  std::vector<int32_t> dump() const { return data; }

 private:
  std::vector<int32_t> data;
};

struct PietStackRBTree {
 public:
  PietStackRBTree() : rbtree_manager(0), data() {}
  void push(int32_t val) { rbtree_manager.push_back(data, val); }
  int32_t pop() {
    if (rbtree_manager.count(data) == 0)
      throw std::out_of_range("the stack is empty");
    return rbtree_manager.pop_back(data);
  }
  void roll(int32_t depth, int32_t count) {
    const auto len = this->size();
    if (depth < 0 || static_cast<size_t>(depth) > len)
      throw std::out_of_range("Invalid depth");
    if (depth == 0) return;  // nothing to do
    count %= depth;
    if (count < 0) count += depth;  // make positive
    auto [left, mid, right] =
        rbtree_manager.split3(data, len - depth, len - count);
    data = rbtree_manager.merge(left, right, mid);
  }
  size_t size() const { return rbtree_manager.count(data); }
  std::vector<int32_t> dump() const { return rbtree_manager.dump(data); }

 private:
  RedBlackTree<int32_t> rbtree_manager;
  std::shared_ptr<RedBlackTree<int32_t>::Node> data;
};

void random_test() {
  std::mt19937 mt(0xdeadbeef);
  std::uniform_int_distribution<int32_t> value_dis(
      std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
  std::discrete_distribution<> initialize_dis({6, 3, 1});  // push, pop, roll
  size_t initialize_steps = 10000;
  PietStack ps_optimized;
  PietStackNaive ps_naive;
  for (size_t i = 0; i < initialize_steps; ++i) {
    switch (initialize_dis(mt)) {
      case 0: {
        const auto val = value_dis(mt);
        ps_optimized.push(val);
        ps_naive.push(val);
      } break;
      case 1: {
        if (ps_optimized.size() == 0) break;
        const auto val_o = ps_optimized.pop();
        const auto val_n = ps_naive.pop();
        if (val_o != val_n) {
          std::cerr << "Value mismatch: " << val_n << " is expected, but got "
                    << val_o << std::endl;
          exit(EXIT_FAILURE);
        }
      } break;
      case 2: {
        std::uniform_int_distribution<size_t> depth_dis(
            0, std::size(ps_optimized));
        const auto depth = depth_dis(mt);
        const auto count = value_dis(mt);
        ps_optimized.roll(depth, count);
        ps_naive.roll(depth, count);
      }
    }
    // sanity check
    if (ps_optimized.size() != ps_naive.size()) {
      std::cerr << "size mismatch" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (ps_optimized.dump() != ps_naive.dump()) {
      std::cerr << "content mismatch" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

template <typename TStack>
double benchmark(size_t size) {
  std::mt19937 mt(0xdeadbeef);
  std::uniform_int_distribution<int32_t> value_dis(
      std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
  std::discrete_distribution<> initialize_dis({6, 3, 1});  // push, pop, roll
  size_t initialize_steps = size;
  TStack ps;
  using clock = std::chrono::system_clock;
  const auto start = clock::now();
  for (size_t i = 0; i < initialize_steps; ++i) {
    switch (initialize_dis(mt)) {
      case 0: {
        const auto val = value_dis(mt);
        ps.push(val);
      } break;
      case 1: {
        if (ps.size() == 0) break;
        ps.pop();
      } break;
      case 2: {
        if (ps.size() == 0) break;
        std::uniform_real_distribution ldepth_dis(0.0, std::log(std::size(ps)));
        auto depth = std::exp(ldepth_dis(mt));
        while (depth > std::size(ps)) depth = std::exp(ldepth_dis(mt));
        const auto count = value_dis(mt);
        ps.roll(depth, count);
      }
    }
  }
  const auto finish = clock::now();
  double elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
          .count();
  return elapsed;
}

int main(int, char** argv) {
  random_test();
  size_t size = std::atoi(argv[1]);
  std::cout << "Naive: " << benchmark<PietStackNaive>(size) << std::endl;
  std::cout << "RBTree: " << benchmark<PietStackRBTree>(size) << std::endl;
  std::cout << "Optimized: " << benchmark<PietStack>(size) << std::endl;
  return 0;
}

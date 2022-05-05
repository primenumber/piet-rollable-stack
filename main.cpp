#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <variant>

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
        rbtree_manager.split3(std::move(data), len - depth, len - count);
    data =
        rbtree_manager.merge(std::move(left), std::move(right), std::move(mid));
  }
  size_t size() const { return rbtree_manager.count(data); }
  std::vector<int32_t> dump() const { return rbtree_manager.dump(data); }

 private:
  RedBlackTree<int32_t> rbtree_manager;
  std::unique_ptr<RedBlackTree<int32_t>::Node> data;
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

struct Push {
  int32_t val;
};

struct Pop {};

struct Roll {
  size_t depth;
  int32_t count;
};

using Query = std::variant<Push, Pop, Roll>;

std::vector<Query> generate_benchmark_input(size_t size) {
  std::mt19937 mt(0xdeadbeef);
  std::uniform_int_distribution<int32_t> value_dis(
      std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
  std::discrete_distribution<> initialize_dis({6, 3, 1});  // push, pop, roll
  size_t stack_size = 0;
  std::vector<Query> queries;
  for (size_t i = 0; i < size; ++i) {
    switch (initialize_dis(mt)) {
      case 0: {
        const auto val = value_dis(mt);
        queries.push_back(Push{val});
        ++stack_size;
      } break;
      case 1: {
        if (stack_size == 0) break;
        queries.push_back(Pop{});
        --stack_size;
      } break;
      case 2: {
        if (stack_size == 0) break;
        std::uniform_real_distribution ldepth_dis(0.0, std::log(stack_size));
        auto depth = std::exp(ldepth_dis(mt));
        while (depth > stack_size) depth = std::exp(ldepth_dis(mt));
        const auto count = value_dis(mt);
        queries.push_back(Roll{depth, count});
      }
    }
  }
  return queries;
}

template <typename TStack>
void update(TStack& stack, Push push) {
  stack.push(push.val);
}

template <typename TStack>
void update(TStack& stack, Pop) {
  stack.pop();
}

template <typename TStack>
void update(TStack& stack, Roll roll) {
  stack.roll(roll.depth, roll.count);
}

template <typename TStack>
double benchmark(const std::vector<Query>& queries) {
  TStack ps;
  using clock = std::chrono::system_clock;
  const auto start = clock::now();
  for (auto& query : queries) {
    std::visit([&](auto&& query) { update(ps, query); }, query);
  }
  const auto finish = clock::now();
  double elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
          .count();
  return elapsed;
}

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "Usage: " << argv[0] << " QUERY_NUMBER" << std::endl;
    exit(EXIT_FAILURE);
  }
  size_t size = std::atoi(argv[1]);
  random_test();
  const auto queries = generate_benchmark_input(size);
  std::cout << "Naive: " << benchmark<PietStackNaive>(queries) << std::endl;
  std::cout << "RBTree: " << benchmark<PietStackRBTree>(queries) << std::endl;
  std::cout << "Optimized: " << benchmark<PietStack>(queries) << std::endl;
  return 0;
}

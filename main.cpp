#include <algorithm>
#include <iostream>
#include <random>

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

void random_test() {
  std::mt19937 mt(0xdeadbeef);
  std::uniform_int_distribution<int32_t> value_dis(
      std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
  std::discrete_distribution<> initialize_dis({2, 1, 1});  // push, pop, roll
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

int main() {
  random_test();
  return 0;
}

#ifndef PIET_ROLLABLE_STACK_STACK_HPP
#define PIET_ROLLABLE_STACK_STACK_HPP
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "rbtree.hpp"

struct PietStack {
 public:
  constexpr static size_t min_head_size_desired = 8;
  PietStack() : head_size_desired(8), head(), rbtree_manager(0), tail() {}
  void push(int32_t val) {
    head.push_back(val);
    if (head.size() >= head_size_desired * 2) {
      for (size_t i = 0; i < head_size_desired; ++i) {
        rbtree_manager.push_back(tail, head[i]);
      }
      head.erase(begin(head), begin(head) + head_size_desired);
      head_size_desired =
          std::max<size_t>(min_head_size_desired, std::ilogb(this->size()));
    }
  }
  int32_t pop() {
    if (head.empty()) {
      if (rbtree_manager.count(tail) == 0)
        throw std::out_of_range("Stack empty");
      head_size_desired =
          std::max<size_t>(min_head_size_desired, std::ilogb(this->size()));
      const size_t take_count =
          std::min<size_t>(rbtree_manager.count(tail), head_size_desired);
      head.resize(take_count);
      for (size_t i = 0; i < take_count; ++i) {
        head[take_count - 1 - i] = rbtree_manager.pop_back(tail);
      }
    }
    auto res = head.back();
    head.pop_back();
    return res;
  }
  void roll(int32_t depth, int32_t count) {
    const auto len = this->size();
    if (depth < 0 || static_cast<size_t>(depth) > len)
      throw std::out_of_range("Invalid depth");
    if (depth == 0) return;  // nothing to do
    count %= depth;
    if (count < 0) count += depth;  // make positive
    if (static_cast<size_t>(depth) <= head_size()) {
      roll_head(depth, count);
      return;
    }
    auto htree = rbtree_manager.build(head);
    tail = rbtree_manager.merge(tail, htree);
    head.clear();
    auto [left, mid, right] =
        rbtree_manager.split3(tail, len - depth, len - count);
    tail = rbtree_manager.merge(left, right, mid);
    head_size_desired =
        std::max<size_t>(min_head_size_desired, std::ilogb(len));
    const size_t take_count =
        std::min<size_t>(rbtree_manager.count(tail), head_size_desired);
    std::tie(tail, htree) = rbtree_manager.split(tail, len - take_count);
    head = rbtree_manager.dump(htree);
  }
  size_t size() const { return head.size() + rbtree_manager.count(tail); }
  size_t head_size() const { return head.size(); }
  size_t tail_size() const { return rbtree_manager.count(tail); }
  std::vector<int32_t> dump() {
    std::vector<int32_t> result = rbtree_manager.dump(tail);
    result.insert(std::end(result), std::begin(head), std::end(head));
    return result;
  }
  std::string to_string() {
    std::string result;
    for (auto&& elem : this->dump()) {
      result += std::to_string(elem);
      result += ", ";
    }
    return result;
  }

 private:
  void roll_head(size_t depth, size_t count) {
    std::rotate(std::end(head) - depth, std::end(head) - count, std::end(head));
  }
  size_t head_size_desired;
  std::vector<int32_t> head;
  RedBlackTree<int32_t> rbtree_manager;
  std::shared_ptr<RedBlackTree<int32_t>::Node> tail;
};

#endif  // PIET_ROLLABLE_STACK_STACK_HPP

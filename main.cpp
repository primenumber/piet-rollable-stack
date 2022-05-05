#include <iostream>

#include "stack.hpp"

int main() {
  PietStack stk;
  for (size_t i = 0; i < 100; ++i) {
    stk.push(i);
  }
  std::cout << stk.head_size() << " " << stk.tail_size() << std::endl;
  std::cout << stk.to_string() << std::endl;
  stk.roll(55, 30);
  std::cout << stk.head_size() << " " << stk.tail_size() << std::endl;
  std::cout << stk.to_string() << std::endl;
  for (size_t i = 0; i < 100; ++i) {
    stk.pop();
  }
}

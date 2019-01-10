//
// Created by marcin on 10/01/19.
//

#include "onemax_fitness_evaluation.hpp"

int onemax_fitness_evaluation::operator()(const std::vector<char> &ind) const noexcept {
  int i{0};
  for (auto b : ind) if (b) ++i;
  return i;
}
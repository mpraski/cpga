//
// Created by marcin on 10/01/19.
//

#include <cpga/examples/onemax/onemax_fitness_evaluation.hpp>

namespace cpga {
using namespace core;
namespace examples {
/**
 * @brief Compute fitness value for a bitstring by summing the ones.
 * @param ind the individual bitstring
 * @return Resulting fitness value
 */
int onemax_fitness_evaluation::operator()(const std::vector<char> &ind) const noexcept {
  int i{0};
  for (auto b : ind) if (b) ++i;
  return i;
}
}
}
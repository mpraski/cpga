//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP
#define GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP

#include "../../core.hpp"

namespace cpga {
using namespace core;
namespace examples {
/**
 * @brief Fitness evaluation genetic operator for onemax problem
 * @details This class defines fitness evaluation for a bitstring (defined
 * by std::vector<char>) by summing the '1' values.
 */
class onemax_fitness_evaluation : public base_operator {
 public:
  using base_operator::base_operator;

  int operator()(const std::vector<char> &ind) const noexcept;
};
}
}

#endif //GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP
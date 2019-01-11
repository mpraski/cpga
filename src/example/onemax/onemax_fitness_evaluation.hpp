//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP
#define GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP

#include "core.hpp"

class onemax_fitness_evaluation : public base_operator<sequence<char>, int> {
  INCLUDES(sequence<char>, int);
 public:
  using base_operator::base_operator;

  int operator()(const std::vector<char> &ind) const noexcept;
};

#endif //GENETIC_ACTOR_ONEMAX_FITNESS_EVALUATION_HPP
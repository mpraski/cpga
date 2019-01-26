//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_BITSTRING_MUTATION_HPP
#define GENETIC_ACTOR_BITSTRING_MUTATION_HPP

#include "core.hpp"

/**
 * @brief Mutation genetic operator for onemax problem
 * @details This class defines mutation for a bitstring (represented by
 * std::vector<char>) as flipping randomly chosen chars.
 */
class bitstring_mutation : public base_operator {
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_f;

  inline void flip(char &c) const noexcept {
    c = c ? 0 : 1;
  }
 public:
  bitstring_mutation() = default;
  bitstring_mutation(const shared_config &config, island_id island_no);

  void operator()(wrapper<sequence<char>, int> &wrapper) const noexcept;
};

#endif //GENETIC_ACTOR_BITSTRING_MUTATION_HPP

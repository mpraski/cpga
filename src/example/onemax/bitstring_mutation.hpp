//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_BITSTRING_MUTATION_HPP
#define GENETIC_ACTOR_BITSTRING_MUTATION_HPP

#include "core.hpp"

class bitstring_mutation : public base_operator<sequence<char>, int> {
  INCLUDES(sequence<char>, int);
 private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
  std::function<double()> random_f;

  inline void flip(char &c) const noexcept {
    if (c == 0) c = 1; else c = 0;
  }
 public:
  bitstring_mutation() = default;
  bitstring_mutation(const shared_config &config, island_id island_no);

  void operator()(wrapper &wrapper) const noexcept;
};

#endif //GENETIC_ACTOR_BITSTRING_MUTATION_HPP

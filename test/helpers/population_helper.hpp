//
// Created by marcin on 10/02/19.
//

#ifndef GENETIC_ACTOR_POPULATION_HELPER_H
#define GENETIC_ACTOR_POPULATION_HELPER_H

#include <cpga/common.hpp>

class population_helper {
 public:
  static cpga::population<int, int> sample_population(size_t sz) {
    cpga::population<int, int> pop(sz);
    std::generate(std::begin(pop), std::end(pop), [c = 1]() mutable {
      auto p = std::make_pair(c, c * 2);
      ++c;
      return p;
    });
    return pop;
  }
};

#endif //GENETIC_ACTOR_POPULATION_HELPER_H

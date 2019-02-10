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

  template<typename Ind, typename FitVal, typename Cons = typename Ind::value_type>
  static bool sequence_population_in_range(const cpga::population<Ind, FitVal> &pop,
                                           const std::vector<Cons> &vals) {
    const auto b = std::begin(vals);
    const auto e = std::end(vals);
    return std::all_of(std::begin(pop), std::end(pop), [&](const auto &wrapper) {
      return std::all_of(std::begin(wrapper.first),
                         std::end(wrapper.first),
                         [&](const auto &val) { return std::find(b, e, val) != e; });
    });
  }
};

#endif //GENETIC_ACTOR_POPULATION_HELPER_H

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

  static cpga::population<int, int> sample_population_with_fitness_value(size_t sz, int average_fitness_value) {
    cpga::population<int, int> pop(sz);
    std::generate(std::begin(pop), std::end(pop), [c = 1, tfv = average_fitness_value * sz]() mutable {
      auto p = std::make_pair(c, tfv);
      ++c;
      if (tfv) tfv = 0;
      return p;
    });
    return pop;
  }

  static cpga::population<std::vector<char>, int> sample_bitstring_population(size_t sz, size_t ind_sz) {
    std::default_random_engine generator;
    std::bernoulli_distribution distribution(0.5);
    cpga::population<std::vector<char>, int> pop(sz);
    std::generate(std::begin(pop), std::end(pop), [&] {
      std::vector<char> ind(ind_sz);
      std::generate(std::begin(ind), std::end(ind), [&] {
        return distribution(generator) ? 0 : 1;
      });
      return std::make_pair(ind, 0);
    });
    return pop;
  }

  template<typename Ind, typename FitVal, typename Cons = typename Ind::value_type>
  static bool sequence_population_in_range(const cpga::population<Ind, FitVal> &pop,
                                           const std::vector<Cons> &vals) {
    const auto b = std::begin(vals), e = std::end(vals);
    return std::all_of(std::begin(pop), std::end(pop), [&](const auto &wrapper) {
      return std::all_of(std::begin(wrapper.first),
                         std::end(wrapper.first),
                         [&](const auto &val) { return std::find(b, e, val) != e; });
    });
  }

  template<typename Ind, typename FitVal, typename Cons = typename Ind::value_type>
  static bool can_be_offspring_of(const cpga::population<Ind, FitVal> &pop,
                                  const cpga::wrapper_pair<Ind, FitVal> &couple) {
    return std::all_of(std::begin(pop), std::end(pop), [&](const auto &wrapper) {
      return std::all_of(std::begin(wrapper.first),
                         std::end(wrapper.first),
                         [&](const auto &val) {
                           return std::find(std::begin(couple.first.first), std::end(couple.first.first), val)
                               != std::end(couple.first.first)
                               || std::find(std::begin(couple.second.first), std::end(couple.second.first), val)
                                   != std::end(couple.second.first);
                         });
    });
  }
};

#endif //GENETIC_ACTOR_POPULATION_HELPER_H

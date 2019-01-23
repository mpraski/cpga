//
// Created by marcin on 20/01/19.
//

#ifndef GENETIC_ACTOR_BEST_MIGRATION_H
#define GENETIC_ACTOR_BEST_MIGRATION_H

#include <random>
#include <vector>
#include <core.hpp>
#include <utilities/population_sorter.hpp>

template<typename individual, typename fitness_value>
class best_migration : public base_operator {
 public:
  using base_operator::base_operator;

  virtual ~best_migration() = default;

  virtual island_id next_destination(
      const wrapper<individual, fitness_value> &wrapper) = 0;

  auto operator()(island_id from, population<individual, fitness_value> &pop) {
    auto &props = config->system_props;

    migration_payload<individual, fitness_value> payload;
    population_sorter<individual, fitness_value>::sort(pop);

    auto quota{std::min(props.migration_quota, pop.size())};
    auto end{std::next(pop.begin(), quota)};
    for (auto it{pop.begin()}; it != end; ++it) {
      payload.emplace_back(next_destination(*it), std::move(*it));
    }

    pop.erase(pop.begin(), end);

    return payload;
  }
};

#endif //GENETIC_ACTOR_BEST_MIGRATION_H
#pragma once

#include "../core.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value>
class random_migration : public base_operator {
  using distribution = std::uniform_int_distribution<size_t>;
 private:
  std::default_random_engine generator;
 public:
  random_migration() = default;
  random_migration(const shared_config &config, island_id island_no)
      : base_operator{config, island_no},
        generator{get_seed(config->system_props.migration_seed)} {
  }

  virtual ~random_migration() = default;

  virtual island_id next_destination(
      const individual_wrapper<individual, fitness_value> &wrapper, size_t population) = 0;

  auto operator()(
      island_id from,
      individual_collection<individual, fitness_value> &population) {
    migration_payload<individual, fitness_value> payload;
    auto quota = config->system_props.migration_quota;

    for (size_t i = 0; i < quota; ++i) {
      auto next = population.begin()
          + distribution{0, population.size() - 1}(generator);
      auto dest = next_destination(*next, population.size());

      payload.emplace_back(dest, std::move(*next));
      population.erase(next);
    }

    return payload;
  }
};

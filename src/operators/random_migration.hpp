#pragma once

#include "../core.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value>
class random_migration : public base_operator {
  using distribution = std::uniform_int_distribution<std::size_t>;
 private:
  std::default_random_engine generator;
  bool add_seed;
 public:
  random_migration() = default;
  random_migration(const shared_config& config, island_id island_no)
      : base_operator { config, island_no },
        add_seed { std::any_cast<bool>(
            config->user_props.at(constants::ADD_POPULATION_SIZE_TO_SEED)) } {
    auto seed = config->system_props.migration_seed;
    if (add_seed) seed += island_no;
    generator = std::default_random_engine { seed };
  }

  virtual ~random_migration() {
  }

  virtual island_id next_destination(
      const individual_wrapper<individual, fitness_value>& wrapper,
      std::size_t no, std::size_t total, std::size_t population) = 0;

  migration_payload<individual, fitness_value> operator()(
      island_id from,
      individual_collection<individual, fitness_value>& population) const {
    migration_payload<individual, fitness_value> payload;
    auto quota = config->system_props.migration_quota;

    for (std::size_t i = 0; i < quota; ++i) {
      auto next = population.begin()
          + distribution { 0, population.size() - 1 }(generator);
      auto dest = next_destination(*next, i, quota, population.size());

      payload.emplace_back(dest, std::move(*next));
      population.erase(next);
    }

    return payload;
  }
};

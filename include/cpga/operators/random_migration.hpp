#ifndef GENETIC_ACTOR_RANDOM_MIGRATION_H
#define GENETIC_ACTOR_RANDOM_MIGRATION_H

#include <random>
#include <vector>
#include "../core.hpp"

namespace cpga {
namespace operators {
/**
 * @brief Genetic operator producing a migration payload for a given island.
 * @details This class performs migration by randomly moving at most system_properties.migration_quota
 * individuals into the payload (and erasing them from population). User must implement the pure virtual
 * method next_destination which specifies the destination island for the n'th migrant.
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class random_migration : public base_operator {
 private:
  std::default_random_engine generator;
 public:
  random_migration() = default;
  random_migration(const shared_config &config, island_id island_no)
      : base_operator{config, island_no},
        generator{get_seed(config->system_props.migration_seed)} {
  }

  virtual ~random_migration() = default;

  /**
   * @brief Specifies destination island for an individual.
   * @param wrapper the individual & fitness value pair
   * @return The destination island for this individual.
   */
  virtual island_id next_destination(const wrapper<individual, fitness_value> &wrapper) = 0;

  /**
 * @brief Builds the migration payload for a given island.
 * @param from the source island id
 * @param pop the source island population
 * @return The migration payload.
 */
  auto operator()(__attribute__((unused)) island_id from, population<individual, fitness_value> &population) {
    migration_payload<individual, fitness_value> payload;
    auto quota = std::min(population.size(), config->system_props.migration_quota);

    for (size_t i = 0; i < quota; ++i) {
      auto next = std::next(
          population.begin(),
          std::uniform_int_distribution<size_t>{0, population.size() - 1}(generator)
      );

      payload.emplace_back(next_destination(*next), std::move(*next));
      population.erase(next);
    }

    return payload;
  }
};
}
}

#endif //GENETIC_ACTOR_RANDOM_MIGRATION_H
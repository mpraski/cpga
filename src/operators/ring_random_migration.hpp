#ifndef GENETIC_ACTOR_RING_RANDOM_MIGRATION_H
#define GENETIC_ACTOR_RING_RANDOM_MIGRATION_H

#include <random_migration.hpp>

/**
 * @brief Genetic operator performing best individual migration using ring topology.
 * @details This implementation of random_migration by sending each individual in the payload
 * to the next immediate island (as if arranged in a ring).
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class ring_random_migration : public random_migration<individual, fitness_value> {
 public:
  using random_migration<individual, fitness_value>::random_migration;

  island_id next_destination(const wrapper<individual, fitness_value> &wrapper) override {
    return (this->island_no + 1) % this->config->system_props.islands_number;
  }
};

#endif //GENETIC_ACTOR_RING_RANDOM_MIGRATION_H
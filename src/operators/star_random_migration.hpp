#ifndef GENETIC_ACTOR_STAR_RANDOM_MIGRATION_H
#define GENETIC_ACTOR_STAR_RANDOM_MIGRATION_H

#include <random_migration.hpp>

/**
 * @brief Genetic operator performing best individual migration using 'star' topology
 * @details This implementation of random_migration by sending each individual in the payload
 * to the next island in the round-robin manner (with the counter incremented every call).
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class star_random_migration : public random_migration<individual, fitness_value> {
 private:
  size_t counter;
 public:
  star_random_migration() = default;
  star_random_migration(const shared_config &config, island_id island_no)
      : random_migration<individual, fitness_value>{config, island_no},
        counter{0} {
    auto &props = this->config->system_props;
    if (props.migration_quota % (props.islands_number - 1) != 0) {
      throw std::runtime_error(
          "Islands number doesn't evenly divide migration quota");
    }
  }

  island_id next_destination(const wrapper<individual, fitness_value> &wrapper) override {
    return (++counter % (this->config->system_props.islands_number - 1));
  }
};

#endif //GENETIC_ACTOR_STAR_RANDOM_MIGRATION_H
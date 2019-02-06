//
// Created by marcin on 20/01/19.
//

#ifndef GENETIC_ACTOR_RING_BEST_MIGRATION_H
#define GENETIC_ACTOR_RING_BEST_MIGRATION_H

#include "best_migration.hpp"

namespace cpga {
namespace operators {
/**
 * @brief Genetic operator performing best individual migration using ring topology.
 * @details This implementation of best_migration by sending each individual in the payload
 * to the next immediate island (as if arranged in a ring).
 * @tparam individual
 * @tparam fitness_value
 */
template<typename individual, typename fitness_value>
class ring_best_migration : public best_migration<individual, fitness_value> {
 public:
  using best_migration<individual, fitness_value>::best_migration;

  island_id next_destination(__attribute__((unused)) const wrapper<individual, fitness_value> &wrapper) override {
    return (this->island_no + 1) % this->config->system_props.islands_number;
  }
};
}
}

#endif //GENETIC_ACTOR_RING_BEST_MIGRATION_H

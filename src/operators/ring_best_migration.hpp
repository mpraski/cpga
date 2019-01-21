//
// Created by marcin on 20/01/19.
//

#ifndef GENETIC_ACTOR_RING_BEST_MIGRATION_H
#define GENETIC_ACTOR_RING_BEST_MIGRATION_H

#include <best_migration.hpp>

template<typename individual, typename fitness_value>
class ring_best_migration : public best_migration<individual, fitness_value> {
 public:
  using best_migration<individual, fitness_value>::best_migration;

  island_id next_destination(const wrapper<individual, fitness_value> &wrapper) override {
    return (this->island_no + 1) % this->config->system_props.islands_number;
  }
};

#endif //GENETIC_ACTOR_RING_BEST_MIGRATION_H

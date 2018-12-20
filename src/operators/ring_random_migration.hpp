#pragma once

#include "random_migration.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value>
class ring_random_migration : public random_migration<individual, fitness_value> {
 public:
  using random_migration<individual, fitness_value>::random_migration;

  island_id next_destination(
      const individual_wrapper<individual, fitness_value> &wrapper, size_t population) override {
    return (this->island_no + 1) % this->config->system_props.islands_number;
  }
};

#pragma once

#include "random_migration.hpp"

#include <random>
#include <vector>

template<typename individual, typename fitness_value>
class star_random_migration : public random_migration<individual, fitness_value> {
 private:
  std::size_t counter;
 public:
  star_random_migration() = default;
  star_random_migration(const shared_config& config, island_id island_no)
      : random_migration<individual, fitness_value> { config, island_no },
        counter { 0 } {
    auto& props = this->config->system_props;
    if (props.migration_quota % (props.islands_number - 1) != 0) {
      throw std::runtime_error(
          "Islands number doesn't evenly divide migration quota");
    }
  }

  island_id next_destination(
      const individual_wrapper<individual, fitness_value>& wrapper,
      std::size_t no, std::size_t total, std::size_t population) override {
    return (++counter % (this->config->system_props.islands_number - 1));
  }
};

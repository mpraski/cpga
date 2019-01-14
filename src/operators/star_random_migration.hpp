#pragma once

#include <random>
#include <vector>
#include <random_migration.hpp>

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

  island_id next_destination(
      const wrapper<individual, fitness_value> &wrapper, size_t population) override {
    return (++counter % (this->config->system_props.islands_number - 1));
  }
};

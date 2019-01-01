//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_OPERATOR_H
#define GENETIC_ACTOR_BASE_OPERATOR_H

#include <common.hpp>
#include "base_state.hpp"

struct base_operator : public base_state {
  base_operator() = default;
  base_operator(const shared_config &config, island_id island_no);
  island_id island_no;

  inline auto get_seed(unsigned long seed) const noexcept {
    if (config->system_props.add_island_no_to_seed) {
      seed += island_no;
    }
    return seed;
  }
};

#endif //GENETIC_ACTOR_BASE_OPERATOR_H

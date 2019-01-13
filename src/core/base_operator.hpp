//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_OPERATOR_H
#define GENETIC_ACTOR_BASE_OPERATOR_H

#include <common.hpp>
#include "base_state.hpp"

class base_operator : public base_state {
 protected:
  inline auto get_seed(unsigned long seed) const noexcept {
    if (config->system_props.add_island_no_to_seed) {
      seed += island_no;
    }
    return seed;
  }
 public:
  base_operator() = default;
  base_operator(const shared_config &config, island_id island_no) : base_state{config},
                                                                    island_no{island_no} {
  }

  island_id island_no;
};

#endif //GENETIC_ACTOR_BASE_OPERATOR_H

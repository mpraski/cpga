//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_OPERATOR_H
#define GENETIC_ACTOR_BASE_OPERATOR_H

#include <common.hpp>
#include "base_state.hpp"

template<typename individual, typename fitness_value>
class base_operator : public base_state {
 protected:
  using wrapper = individual_wrapper<individual, fitness_value>;
  using wrapper_pair = individual_wrapper_pair<individual, fitness_value>;
  using population = individual_collection<individual, fitness_value>;
  using parents = parent_collection<individual, fitness_value>;
  using inserter = individual_collection_inserter<individual, fitness_value>;
 public:
  base_operator() = default;
  base_operator(const shared_config &config, island_id island_no) : base_state{config},
                                                                    island_no{island_no} {
  }

  inline auto get_seed(unsigned long seed) const noexcept {
    if (config->system_props.add_island_no_to_seed) {
      seed += island_no;
    }
    return seed;
  }

  island_id island_no;
};

#define INCLUDES(IND, FIT) \
using typename base_operator<IND, FIT>::wrapper; \
using typename base_operator<IND, FIT>::wrapper_pair; \
using typename base_operator<IND, FIT>::population; \
using typename base_operator<IND, FIT>::parents; \
using typename base_operator<IND, FIT>::inserter; \
using base_operator<IND, FIT>::get_seed; \
using base_operator<IND, FIT>::config; \

#endif //GENETIC_ACTOR_BASE_OPERATOR_H

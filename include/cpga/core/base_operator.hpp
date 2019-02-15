//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_BASE_OPERATOR_H
#define GENETIC_ACTOR_BASE_OPERATOR_H

#include "../common.hpp"
#include "base_state.hpp"

namespace cpga {
namespace core {
/**
 * @brief This is the intended superclass for all genetic operators used by CPGA.
 */
class base_operator : public base_state {
 protected:
  /**
   * @brief Helper method for obtaining a seed augmented with island id defined for this operator.
   * @param seed The base value of a seed
   * @return The base value with added island number
   */
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

  /**
   * @brief Each genetic operator knows the id of the 'containing' island, which
   * is only really meaningful when ISLAND model is executed. In other models it is set to 0.
   */
  island_id island_no;
};
}
}

#endif //GENETIC_ACTOR_BASE_OPERATOR_H

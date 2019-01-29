//
// Created by marcin on 23/01/19.
//

#ifndef GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H
#define GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H

#include <caf/all.hpp>
#include "data.hpp"

/**
 * @brief This class provides a simple way to bootstrap a PGA model on a single machine.
 * @tparam driver the subclass of base_single_machine_driver<individual, fitness_value>, defining behaviour of a PGA run on a single computer
 */
template<typename driver>
class single_machine_runner {
 public:
  /**
   * @brief Start execution of a single machine.
   * @param system the CAF actor system
   * @param system_props the required base PGA configuration
   * @param user_props the optional user-defined configuration used by custom genetic operators
   */
  static void run(actor_system &system,
                  const system_properties &system_props,
                  const user_properties &user_props) {
    driver dr{system, system_props, user_props};
    dr.run(system);
  }
};

#endif //GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H

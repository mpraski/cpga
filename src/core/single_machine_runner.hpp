//
// Created by marcin on 23/01/19.
//

#ifndef GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H
#define GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H

#include <caf/all.hpp>
#include "data.hpp"

template<typename driver>
class single_machine_runner {
 public:
  static void run(actor_system &system,
                  const system_properties &system_props,
                  const user_properties &user_props) {
    driver dr{system, system_props, user_props};
    dr.run(system);
  }
};

#endif //GENETIC_ACTOR_SINGLE_MACHINE_RUNNER_H

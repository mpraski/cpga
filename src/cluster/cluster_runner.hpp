//
// Created by marcin on 01/01/19.
//

#ifndef GENETIC_ACTOR_CLUSTER_RUNNER_H
#define GENETIC_ACTOR_CLUSTER_RUNNER_H

#include <core.hpp>

template<typename master_driver, typename worker_driver, typename reporter_driver>
class cluster_runner {
 public:
  static void run(actor_system &system,
                  const system_properties &system_props,
                  const user_properties &user_props,
                  const cluster_properties &cluster_props) {
    std::unique_ptr<base_cluster_driver> driver;
    switch (cluster_props.mode()) {
      case cluster_mode::MASTER:driver.reset(new master_driver{system_props, user_props, cluster_props});
        break;
      case cluster_mode::WORKER:driver.reset(new worker_driver{system_props, user_props, cluster_props});
        break;
      case cluster_mode::REPORTER:driver.reset(new reporter_driver{system_props, user_props, cluster_props});
        break;
    }
    driver->run(system);
  }
};

#endif //GENETIC_ACTOR_CLUSTER_RUNNER_H

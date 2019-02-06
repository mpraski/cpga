//
// Created by marcin on 01/01/19.
//

#ifndef GENETIC_ACTOR_CLUSTER_RUNNER_H
#define GENETIC_ACTOR_CLUSTER_RUNNER_H

#include <core.hpp>

namespace cpga {
using namespace core;
namespace cluster {
/**
 * @brief This class provides a simple way to bootstrap a PGA model on a cluster.
 * @details The user supplies classes defining behaviour of the three node types and calls run to
 * start the execution of a single node, specified by the command line parameters passed via cluster_properties.
 * @tparam master_driver subclass of master_node_driver, implementing logic for the master node
 * @tparam worker_driver subclass of worker_node_driver, implementing logic for the worker nodes
 * @tparam reporter_driver subclass of reporter_node_driver<individual, fitness_value>, implementing logic for the reporter node
 */
template<typename master_driver, typename worker_driver, typename reporter_driver>
class cluster_runner {
 public:
  /**
   * @brief Start execution of a single PGA cluster node.
   * @details The node to be run is specified by a call to cluster_properties.mode().
   * @param system the CAF actor system
   * @param system_props the required base PGA configuration
   * @param user_props the optional user-defined configuration used by custom genetic operators
   * @param cluster_props the configuration object derived from command line parameters
   * @see cluster_properties
   */
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
}
}

#endif //GENETIC_ACTOR_CLUSTER_RUNNER_H

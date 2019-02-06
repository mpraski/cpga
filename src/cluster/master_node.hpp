//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_MASTER_NODE_H
#define GENETIC_ACTOR_MASTER_NODE_H

#include <core.hpp>

namespace cpga {
using namespace core;
namespace cluster {
struct master_node_state : public base_cluster_state {
  master_node_state() = default;
  explicit master_node_state(const cluster_properties &props);

  reporter_node_info reporter_info;
  std::vector<worker_node_info> workers_info;

  size_t current_worker_pings;
  size_t current_worker_infos;

  bool workers_online;
  bool reporter_info_received;
};

behavior master_node(stateful_actor<master_node_state> *self,
                     master_node_state state,
                     const group &workers_group,
                     const actor &executor);

using actor_spawner = std::function<actor(stateful_actor<base_state> *self, std::vector<actor> &)>;

behavior master_node_executor(stateful_actor<base_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const group &message_bus_group,
                              const actor_spawner &factory);

class master_node_driver : public base_cluster_driver {
 protected:
  virtual actor spawn_executor(stateful_actor<base_state> *self, std::vector<actor> &workers) = 0;
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override;
};
}
}

#endif //GENETIC_ACTOR_MASTER_NODE_H

//
// Created by marcin on 31/12/18.
//

#ifndef GENETIC_ACTOR_WORKER_NODE_H
#define GENETIC_ACTOR_WORKER_NODE_H

#include <core.hpp>

struct worker_node_executor_state : public base_state {
  worker_node_executor_state() = default;
  explicit worker_node_executor_state(const shared_config &config) : base_state{config}, workers_counter{0} {}

  size_t workers_counter;
};

using worker_spawner = std::function<std::vector<uint16_t>(stateful_actor<worker_node_executor_state> *self)>;

behavior worker_node_executor(stateful_actor<worker_node_executor_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const cluster_properties &cluster_props,
                              const actor &master_node,
                              const group &message_bus_group,
                              const worker_spawner &factory);

behavior worker_node(event_based_actor *self,
                     const actor &master_node,
                     const actor &executor1,
                     const group &node_group);

class worker_node_driver : public base_cluster_driver {
 protected:
  virtual std::vector<uint16_t> spawn_workers(stateful_actor<worker_node_executor_state> *self) = 0;
 public:
  using base_cluster_driver::base_cluster_driver;

  virtual ~worker_node_driver() = default;

  void perform(scoped_actor &self) override;
};

#endif //GENETIC_ACTOR_WORKER_NODE_H

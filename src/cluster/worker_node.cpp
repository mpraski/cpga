//
// Created by marcin on 31/12/18.
//

#include "worker_node.hpp"

behavior worker_node_executor(stateful_actor<worker_node_executor_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const cluster_properties &cluster_props,
                              const actor &master_node,
                              const worker_spawner &factory) {
  return {
      [=](const reporter_node_info &reporter_info) {
        auto &system = self->system();

        auto[generation_reporter, individual_reporter, system_reporter] = bind_remote_reporters(system, reporter_info);
        auto config = make_shared_config(
            system_props,
            user_props,
            generation_reporter,
            individual_reporter,
            system_reporter
        );

        self->state = worker_node_executor_state{config};

        self->send(master_node,
                   stage_discover_workers::value,
                   worker_node_info{cluster_props.this_node_host, factory(self)});
      }
  };
}

behavior worker_node(event_based_actor *self,
                     const actor &master_node,
                     const actor &executor,
                     const group &node_group) {
  self->join(node_group);

  return {
      [=](stage_initiate_worker_node) {
        self->send(master_node, stage_collect_workers::value);
      },
      [=](reporter_node_info &info) {
        self->send(executor, std::move(info));
      },
      [=](const group_down_msg &) {
      }
  };
}

void worker_node_driver::perform(scoped_actor &self) {
  using namespace std::placeholders;

  auto &system = self->system();
  auto &middleman = system.middleman();
  auto master_node = wait_for_master_node(middleman);
  auto node_group = wait_for_node_group(system);

  auto executor =
      self->spawn<detached>(worker_node_executor,
                            system_props,
                            user_props,
                            cluster_props,
                            master_node,
                            std::bind(&worker_node_driver::spawn_workers, this, _1));

  auto node = self->spawn<detached>(worker_node, master_node, executor, node_group);

  anon_send(node, stage_initiate_worker_node::value);
  self->wait_for(executor);
  anon_send_exit(node, exit_reason::user_shutdown);
}
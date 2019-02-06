//
// Created by marcin on 01/01/19.
//

#include <csignal>
#include "master_node.hpp"

using namespace cpga::cluster;
using namespace cpga::core;

master_node_state::master_node_state(const cluster_properties &props) : base_cluster_state{props},
                                                                        current_worker_pings{0},
                                                                        current_worker_infos{0},
                                                                        workers_online{false},
                                                                        reporter_info_received{false} {}

behavior master_node(stateful_actor<master_node_state> *self,
                     master_node_state state,
                     const group &workers_group,
                     const actor &executor) {
  self->state = std::move(state);
  self->join(workers_group);

  log(self, "** Beginning node discovery **");

  return {
      [=](stage_discover_reporters, reporter_node_info &info) {
        auto &state = self->state;
        log(self, "Discovered reporters: ", info);
        state.reporter_info = std::move(info);
        state.reporter_info_received = true;
        if (state.workers_online && state.reporter_info_received) {
          self->send(self, stage_distribute_reporter_info::value);
        }
      },
      [=](stage_collect_workers) {
        auto &state = self->state;
        log(self,
            "Collected workers from a node. ",
            (state.cluster_props.expected_worker_nodes - state.current_worker_pings - 1),
            " more nodes to go");
        if (++state.current_worker_pings == state.cluster_props.expected_worker_nodes) {
          state.workers_online = true;
          log(self, "Collected workers from all nodes");
          if (state.workers_online && state.reporter_info_received) {
            self->send(self, stage_distribute_reporter_info::value);
          }
        }
      },
      [=](stage_distribute_reporter_info) {
        self->send(workers_group, self->state.reporter_info);
        log(self, "Distributed reporters info");
      },
      [=](stage_discover_workers, worker_node_info &info) {
        auto &state = self->state;
        log(self, "Discovered workers: ", info);
        state.workers_info.emplace_back(std::move(info));
        if (++state.current_worker_infos == state.cluster_props.expected_worker_nodes) {
          state.current_worker_infos = 0;
          self->send(executor, state.reporter_info, state.workers_info);
        }
      },
      [](const reporter_node_info &) {}
  };
}

behavior master_node_executor(stateful_actor<base_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const group &message_bus_group,
                              const actor_spawner &factory) {
  return {
      [=](const reporter_node_info &reporter_info, const std::vector<worker_node_info> &workers_info) {
        auto &system = self->system();

        auto[generation_reporter, individual_reporter, system_reporter] = bind_remote_reporters(system, reporter_info);
        auto workers = bind_remote_workers(system, workers_info);

        auto sys_props{system_props};
        if (workers.size() != system_props.islands_number) {
          system_message(self,
                         "Actual number of islands/workers (",
                         workers.size(),
                         ") differs from declared one (",
                         system_props.islands_number,
                         "). Adjusting system configuration...");
          sys_props.islands_number = workers.size();
          sys_props.compute_population_size();
        }

        auto config = make_shared_config(
            sys_props,
            user_props,
            generation_reporter,
            individual_reporter,
            system_reporter,
            message_bus{message_bus_group}
        );

        self->state = base_state{config};

        for (const auto &worker : workers) {
          self->monitor(worker);
        }

        show_model_info(self, sys_props);

        auto executor = factory(self, workers);

        self->set_down_handler([=](const down_msg &down) {
          if (down.source == executor) {
            if (sys_props.is_system_reporter_active) {
              system_message(self, "Quitting reporters");

              self->send(config->system_reporter, exit_reporter::value);
            }

            if (sys_props.is_generation_reporter_active) {
              self->send(config->generation_reporter, exit_reporter::value);
            }

            if (sys_props.is_individual_reporter_active) {
              self->send(config->individual_reporter, exit_reporter::value);
            }

            self->quit();
          }
        });
      },
  };
}

void master_node_driver::perform(scoped_actor &self) {
  using namespace std::placeholders;

  auto &system = self->system();
  auto &middleman = system.middleman();
  auto node_group = system.groups().get_local(constants::NODE_GROUP);
  auto message_bus_group = system.groups().get_local(constants::MESSAGE_BUS_GROUP);
  auto executor =
      self->spawn<detached>(master_node_executor,
                            system_props,
                            user_props,
                            message_bus_group,
                            std::bind(&master_node_driver::spawn_executor, this, _1, _2));
  auto node = self->spawn<detached>(master_node, master_node_state{cluster_props}, node_group, executor);

  middleman.publish_local_groups(cluster_props.master_group_port);
  if (auto published{middleman.publish(node, cluster_props.master_node_port)}; !published) {
    throw std::runtime_error(str("unable to publish master node: ", system.render(published.error())));
  }

  self->wait_for(executor);
  anon_send_exit(node, exit_reason::user_shutdown);
}
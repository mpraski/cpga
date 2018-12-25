//
// Created by marcin on 23/12/18.
//

#ifndef GENETIC_ACTOR_MASTER_NODE_H
#define GENETIC_ACTOR_MASTER_NODE_H

#include <utilities/finite_state_machine.hpp>
#include <caf/all.hpp>
#include <caf/io/all.hpp>
#include <atoms.hpp>
#include <core.hpp>
#include <global_model.hpp>

using namespace caf;
using namespace caf::io;

namespace cgf {
namespace cluster {
struct master_node_state : public base_cluster_state {
  master_node_state() = default;
  master_node_state(const cluster_properties &props) : base_cluster_state{props},
                                                       current_worker_pings{0},
                                                       current_worker_infos{0},
                                                       workers_online{false},
                                                       reporter_info_received{false} {
    workers_info.reserve(props.expected_worker_nodes);
  }

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
                     const actor &executor) {
  self->state = std::move(state);
  self->join(workers_group);

  return {
      [=](stage_discover_reporters, reporter_node_info &info) {
        auto &state = self->state;
        state.reporter_info = std::move(info);
        state.reporter_info_received = true;

        if (state.workers_online && state.reporter_info_received) {
          self->send(self, stage_distribute_reporter_info::value);
        }
      },
      [=](stage_collect_workers) {
        if (self == self->current_sender()) return;

        auto &state = self->state;
        if (++state.current_worker_pings == state.cluster_props.expected_worker_nodes) {
          state.workers_online = true;
          if (state.workers_online && state.reporter_info_received) {
            self->send(self, stage_distribute_reporter_info::value);
          }
        }
      },
      [=](stage_distribute_reporter_info) {
        self->send(workers_group, self->state.reporter_info);
      },
      [=](stage_discover_workers, worker_node_info &info) {
        auto &state = self->state;
        state.workers_info.emplace_back(std::move(info));

        if (++state.current_worker_infos == state.cluster_props.expected_worker_nodes) {
          state.current_worker_infos = 0;
          self->send(executor, state.reporter_info, state.workers_info);
        }
      }
  };
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator,
    typename global_termination_check>
void master_node_executor(blocking_actor *self,
                          const system_properties &system_props,
                          const user_properties &user_props) {
  volatile bool running = true;
  self->receive_while([&] { return running; })(
      [&](const reporter_node_info &reporter_info, const std::vector<worker_node_info> &workers_info) {
        auto &system = self->system();

        auto[generation_rep, individual_rep, system_rep] = bind_remote_reporters(system, reporter_info);
        auto workers = bind_remote_workers(system, workers_info);
        auto config = make_shared_config(
            system_props,
            user_props,
            generation_rep,
            individual_rep,
            system_rep
        );

        auto supervisor = system.spawn(
            global_model_supervisor<individual, fitness_value,
                                    fitness_evaluation_operator>,
            global_model_supervisor_state{config, workers});

        auto executor = system.spawn(
            global_model_executor<individual, fitness_value,
                                  initialization_operator, crossover_operator, mutation_operator,
                                  parent_selection_operator, global_termination_check,
                                  survival_selection_operator, elitism_operator>,
            global_model_executor_state<individual, fitness_value,
                                        initialization_operator, crossover_operator, mutation_operator,
                                        parent_selection_operator, global_termination_check,
                                        survival_selection_operator, elitism_operator>{config},
            supervisor);

        self->send(executor, init_population::value);
        self->wait_for(executor, supervisor);
      },
      [&](exit_msg &msg) {
        if (msg.reason == exit_reason::kill) running = false;
      },
      others >> [](message_view &x) -> result<message> {
        return sec::unexpected_message;
      }
  );
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>,
    typename global_termination_check = default_global_termination_check<
        individual, fitness_value>>
class master_node_driver : public base_cluster_driver {
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override {
    auto &system = self->system();
    auto node_group = system.groups().get_local("node_group");
    auto &executor_fun = master_node_executor<individual,
                                              fitness_value,
                                              fitness_evaluation_operator,
                                              initialization_operator,
                                              crossover_operator,
                                              mutation_operator,
                                              parent_selection_operator,
                                              survival_selection_operator,
                                              elitism_operator,
                                              global_termination_check>;
    auto executor = self->spawn<detached>(executor_fun, system_props, user_props);
    auto node = self->spawn<detached>(master_node, master_node_state{}, node_group, executor);
    auto &middleman = system.middleman();

    middleman.publish_local_groups(cluster_props.master_node_port);
    if (auto published{middleman.publish(node, cluster_props.master_group_port)}; !published) {
      throw std::runtime_error(str("unable to publish master node: ", system.render(published.error())));
    }

    self->wait_for(executor);
  }
};

template<typename individual, typename fitness_value>
class reporter_node_driver : public base_cluster_driver {
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override {
    auto &system = self->system();
    auto &middleman = system.middleman();
    auto port_factory = reporter_port_factory();
    reporter_node_info info{""};

    if (system_props.is_system_reporter_active) {
      if (system_props.system_reporter_log.empty()) {
        throw std::runtime_error("system_reporter_log is empty");
      }

      auto actor = self->spawn(system_reporter);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.system_reporter_log, constants::SYSTEM_HEADERS);

        system_message(self, actor, "Spawning reporters");

        info.system_reporter_port = port;
      }
    }

    if (system_props.is_generation_reporter_active) {
      if (system_props.generation_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      auto actor = system.spawn(time_reporter);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.generation_reporter_log, constants::TIME_HEADERS);

        info.system_reporter_port = port;
      }
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      auto actor = system.spawn(
          individual_reporter<individual, fitness_value>);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.individual_reporter_log,
                   constants::INDIVIDUAL_HEADERS);

        info.system_reporter_port = port;
      }
    }

    if (auto master_node
          {middleman.remote_actor(cluster_props.master_node_host, cluster_props.master_node_port)}; !master_node) {
      throw std::runtime_error(str("unable to connect to master node: ", system.render(master_node.error())));
    } else {
      self->send(*master_node, stage_discover_reporters::value, info);
    }

    self->await_all_other_actors_done();
  }
};
}
}

#endif //GENETIC_ACTOR_MASTER_NODE_H

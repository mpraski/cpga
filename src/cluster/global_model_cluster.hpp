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
  explicit master_node_state(const cluster_properties &props) : base_cluster_state{props},
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
        log(self, "Discovered reporters: ", info);
        state.reporter_info = std::move(info);
        state.reporter_info_received = true;
        if (state.workers_online && state.reporter_info_received) {
          self->send(self, stage_distribute_reporter_info::value);
        }
      },
      [=](stage_collect_workers) {
        auto &state = self->state;
        log(self, "Collected workers from node ", state.current_worker_pings);
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
      [=](const reporter_node_info &) {
      },
      [=](stage_discover_workers, worker_node_info &info) {
        auto &state = self->state;
        log(self, "Discovered workers: ", info);
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
behavior master_node_executor(stateful_actor<base_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props) {
  return {
      [=](const reporter_node_info &reporter_info, const std::vector<worker_node_info> &workers_info) {
        auto &system = self->system();

        auto[generation_reporter, individual_reporter, system_reporter] = bind_remote_reporters(system, reporter_info);
        auto workers = bind_remote_workers(system, workers_info);
        auto config = make_shared_config(
            system_props,
            user_props,
            generation_reporter,
            individual_reporter,
            system_reporter
        );

        self->state = base_state{config};

        for (const auto &worker : workers) {
          self->monitor(worker);
        }

        auto supervisor = self->spawn<detached>(
            global_model_supervisor<individual, fitness_value,
                                    fitness_evaluation_operator>,
            global_model_supervisor_state{config, workers});

        auto executor = self->spawn<detached + monitored>(
            global_model_executor<individual, fitness_value,
                                  initialization_operator, crossover_operator, mutation_operator,
                                  parent_selection_operator, global_termination_check,
                                  survival_selection_operator, elitism_operator>,
            global_model_executor_state<individual, fitness_value,
                                        initialization_operator, crossover_operator, mutation_operator,
                                        parent_selection_operator, global_termination_check,
                                        survival_selection_operator, elitism_operator>{config},
            supervisor);

        self->set_down_handler([=](const down_msg &down) {
          if (down.source == executor) {
            if (system_props.is_system_reporter_active) {
              system_message(self, "Quitting reporters");

              self->send(config->system_reporter, exit_reporter::value);
            }

            if (system_props.is_generation_reporter_active) {
              self->send(config->generation_reporter, exit_reporter::value);
            }

            if (system_props.is_individual_reporter_active) {
              self->send(config->individual_reporter, exit_reporter::value);
            }

            self->quit();
          }
        });

        self->send(executor, init_population::value);
      },
  };
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
    auto &middleman = system.middleman();
    auto node_group = system.groups().get_local(constants::NODE_GROUP);
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
    auto node = self->spawn<detached>(master_node, master_node_state{cluster_props}, node_group, executor);

    middleman.publish_local_groups(cluster_props.master_group_port);
    if (auto published{middleman.publish(node, cluster_props.master_node_port)}; !published) {
      throw std::runtime_error(str("unable to publish master node: ", system.render(published.error())));
    }

    self->wait_for(executor);
    anon_send_exit(node, exit_reason::user_shutdown);
  }
};

template<typename individual, typename fitness_value>
class reporter_node_driver : public base_cluster_driver {
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override {
    auto &system = self->system();
    auto &middleman = system.middleman();
    auto master_node = wait_for_master_node(middleman);
    auto port_factory = make_reporter_port_factory();

    reporter_node_info info{"localhost"};

    actor sys_rep;
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

        info.system_reporter_port = port;
        sys_rep = std::move(actor);
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

        info.generation_reporter_port = port;
      }
    }

    if (system_props.is_individual_reporter_active) {
      if (system_props.individual_reporter_log.empty()) {
        throw std::runtime_error("actor_reporter_log is empty");
      }

      auto actor = system.spawn(individual_reporter<individual, fitness_value>);
      auto port = port_factory();
      if (auto published{middleman.publish(actor, port)}; !published) {
        throw std::runtime_error(str("unable to publish system reporter: ", system.render(published.error())));
      } else {
        self->send(actor, init_reporter::value,
                   system_props.individual_reporter_log,
                   constants::INDIVIDUAL_HEADERS);

        info.individual_reporter_port = port;
      }
    }

    if (system_props.is_system_reporter_active) {
      system_message(self, sys_rep, "Spawned reporters: ", info);
    }

    self->send(master_node, stage_discover_reporters::value, info);
    self->await_all_other_actors_done();
  }
};

struct worker_node_executor_state : public base_state {
  worker_node_executor_state() = default;
  explicit worker_node_executor_state(const shared_config &config) : base_state{config}, workers_counter{0} {}

  size_t workers_counter;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator>
behavior worker_node_executor(stateful_actor<worker_node_executor_state> *self,
                              const system_properties &system_props,
                              const user_properties &user_props,
                              const actor &master_node,
                              const std::function<uint16_t()> &port_factory) {
  return {
      [=](const reporter_node_info &reporter_info) {
        auto &system = self->system();
        auto &middleman = system.middleman();

        auto[generation_reporter, individual_reporter, system_reporter] = bind_remote_reporters(system, reporter_info);
        auto config = make_shared_config(
            system_props,
            user_props,
            generation_reporter,
            individual_reporter,
            system_reporter
        );

        self->state = worker_node_executor_state{config};

        std::vector<actor> workers(system_props.islands_number);
        auto spawn_worker = [&] {
          auto worker = self->spawn<detached + monitored>(global_model_worker<individual,
                                                                              fitness_value,
                                                                              fitness_evaluation_operator>,
                                                          global_model_worker_state<fitness_evaluation_operator>{
                                                              config});
          system_message(self, "Spawning worker (actor id: ", worker.id(), ")");
          return worker;
        };
        std::generate(std::begin(workers), std::end(workers), spawn_worker);

        self->set_down_handler([=](const down_msg &down) {
          if (std::any_of(std::begin(workers),
                          std::end(workers),
                          [src = down.source](const auto &worker) { return worker == src; })
              && ++self->state.workers_counter == workers.size()) {
            self->quit();
          }
        });

        std::vector<uint16_t> ports;
        auto publish_worker = [&](const actor &worker) -> uint16_t {
          auto port = port_factory();
          if (auto published{middleman.publish(worker, port)}; !published) {
            throw std::runtime_error(str("unable to publish global model worker: ", system.render(published.error())));
          }
          system_message(self, "Publishing worker (actor id: ", worker.id(), ") on port ", port);
          return port;
        };
        std::transform(std::begin(workers), std::end(workers), std::back_inserter(ports), publish_worker);

        self->send(master_node, stage_discover_workers::value, worker_node_info{"localhost", std::move(ports)});
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

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator>
class worker_node_driver : public base_cluster_driver {
 public:
  using base_cluster_driver::base_cluster_driver;

  void perform(scoped_actor &self) override {
    auto &system = self->system();
    auto &middleman = system.middleman();
    auto master_node = wait_for_master_node(middleman);
    auto node_group = wait_for_node_group(system);
    auto port_factory = make_worker_port_factory();

    auto executor =
        self->spawn<detached>(worker_node_executor<individual, fitness_value, fitness_evaluation_operator>,
                              system_props,
                              user_props,
                              master_node,
                              port_factory);

    auto node = self->spawn<detached>(worker_node, master_node, executor, node_group);

    anon_send(node, stage_initiate_worker_node::value);
    self->wait_for(executor);
    anon_send_exit(node, exit_reason::user_shutdown);
  }
};

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
void run_global_model(actor_system &system,
                      const system_properties &system_props,
                      const user_properties &user_props,
                      const cluster_properties &cluster_props) {
  std::unique_ptr<base_cluster_driver> driver;
  switch (cluster_props.mode()) {
    case cluster_mode::MASTER:
      driver.reset(new master_node_driver<individual,
                                          fitness_value,
                                          fitness_evaluation_operator,
                                          initialization_operator,
                                          crossover_operator,
                                          mutation_operator,
                                          parent_selection_operator,
                                          survival_selection_operator,
                                          elitism_operator,
                                          global_termination_check>{system_props, user_props, cluster_props});
      break;
    case cluster_mode::WORKER:
      driver.reset(new worker_node_driver<individual,
                                          fitness_value,
                                          fitness_evaluation_operator>{system_props, user_props, cluster_props});
      break;
    case cluster_mode::REPORTER:
      driver.reset(new reporter_node_driver<individual, fitness_value>{system_props, user_props, cluster_props});
      break;
  }
  driver->run(system);
}
}
}

#endif //GENETIC_ACTOR_MASTER_NODE_H

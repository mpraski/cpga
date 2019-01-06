//
// Created by marcin on 02/01/19.
//

#ifndef GENETIC_ACTOR_ISLAND_MODEL_CLUSTER_H
#define GENETIC_ACTOR_ISLAND_MODEL_CLUSTER_H

#include <caf/all.hpp>
#include <caf/io/all.hpp>
#include <atoms.hpp>
#include <core.hpp>
#include <island_model.hpp>
#include <cluster.hpp>

using namespace caf;
using namespace caf::io;

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator,
    typename migration_operator>
class island_master_node_driver : public master_node_driver {
 public:
  using master_node_driver::master_node_driver;

  actor spawn_executor(stateful_actor<base_state> *self, std::vector<actor> &workers) override {
    auto &config = self->state.config;
    auto &system = self->system();

    auto &dispatcher_func = island_model_dispatcher<individual, fitness_value,
                                                    fitness_evaluation_operator, initialization_operator,
                                                    crossover_operator, mutation_operator, parent_selection_operator,
                                                    survival_selection_operator, elitism_operator, migration_operator>;

    auto dispatcher = self->spawn<detached>(dispatcher_func,
                                            island_model_dispatcher_state{config, workers});

    auto executor = self->spawn<detached + monitored>(island_model_executor,
                                                      island_model_executor_state{config},
                                                      dispatcher);

    self->send(executor, execute_phase_1::value);

    return executor;
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator,
    typename migration_operator>
class island_worker_node_driver : public worker_node_driver {
 public:
  using worker_node_driver::worker_node_driver;

  std::vector<uint16_t> spawn_workers(stateful_actor<worker_node_executor_state> *self) override {
    auto &system = self->system();
    auto &middleman = system.middleman();
    auto &config = self->state.config;
    auto port_factory = make_worker_port_factory();

    std::vector<actor> workers(system_props.islands_number);
    auto spawn_worker = [&] {
      auto &worker_fun = island_model_worker<individual, fitness_value,
                                             fitness_evaluation_operator,
                                             initialization_operator,
                                             crossover_operator,
                                             mutation_operator,
                                             parent_selection_operator,
                                             survival_selection_operator,
                                             elitism_operator,
                                             migration_operator>;

      using worker_state = island_model_worker_state<individual, fitness_value,
                                                     fitness_evaluation_operator,
                                                     initialization_operator,
                                                     crossover_operator,
                                                     mutation_operator,
                                                     parent_selection_operator,
                                                     survival_selection_operator,
                                                     elitism_operator,
                                                     migration_operator>;

      return self->template spawn<monitored + detached>(worker_fun, config);
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
        throw std::runtime_error(str("unable to publish island model worker: ", system.render(published.error())));
      }
      system_message(self, "Publishing worker (actor id: ", worker.id(), ") on port ", port);
      return port;
    };
    std::transform(std::begin(workers), std::end(workers), std::back_inserter(ports), publish_worker);

    return ports;
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>,
    typename migration_operator = default_migration_operator<individual,
                                                             fitness_value>>
using island_cluster_runner = cluster_runner<island_master_node_driver<individual,
                                                                       fitness_value,
                                                                       fitness_evaluation_operator,
                                                                       initialization_operator,
                                                                       crossover_operator,
                                                                       mutation_operator,
                                                                       parent_selection_operator,
                                                                       survival_selection_operator,
                                                                       elitism_operator,
                                                                       migration_operator>,
                                             island_worker_node_driver<individual,
                                                                       fitness_value,
                                                                       fitness_evaluation_operator,
                                                                       initialization_operator,
                                                                       crossover_operator,
                                                                       mutation_operator,
                                                                       parent_selection_operator,
                                                                       survival_selection_operator,
                                                                       elitism_operator,
                                                                       migration_operator>,
                                             reporter_node_driver<individual, fitness_value>>;
#endif //GENETIC_ACTOR_ISLAND_MODEL_CLUSTER_H

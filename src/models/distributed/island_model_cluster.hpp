//
// Created by marcin on 02/01/19.
//

#ifndef GENETIC_ACTOR_ISLAND_MODEL_CLUSTER_H
#define GENETIC_ACTOR_ISLAND_MODEL_CLUSTER_H

#include <atoms.hpp>
#include <core.hpp>
#include <island_model.hpp>
#include <cluster.hpp>

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

  std::vector<actor> spawn_workers(stateful_actor<worker_node_executor_state> *self) override {
    auto &config = self->state.config;

    std::vector<actor> workers(system_props.islands_number);
    auto spawn_worker = [&] {
      return self->template spawn<monitored + detached>(island_model_worker<individual, fitness_value,
                                                                            fitness_evaluation_operator,
                                                                            initialization_operator,
                                                                            crossover_operator,
                                                                            mutation_operator,
                                                                            parent_selection_operator,
                                                                            survival_selection_operator,
                                                                            elitism_operator,
                                                                            migration_operator>,
                                                        config);
    };
    std::generate(std::begin(workers), std::end(workers), spawn_worker);

    return workers;
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

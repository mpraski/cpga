//
// Created by marcin on 23/12/18.
//

#ifndef GENETIC_ACTOR_GLOBAL_MODEL_CLUSTER_H
#define GENETIC_ACTOR_GLOBAL_MODEL_CLUSTER_H

#include <atoms.hpp>
#include <core.hpp>
#include <global_model.hpp>
#include <cluster.hpp>

namespace cpga {
using namespace cluster;
namespace models {
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator,
    typename global_termination_check>
class global_master_node_driver : public master_node_driver {
 public:
  using master_node_driver::master_node_driver;

  actor spawn_executor(stateful_actor<base_state> *self, std::vector<actor> &workers) override {
      auto &config = self->state.config;

      auto supervisor = self->spawn<detached>(
          global_model_supervisor<individual, fitness_value>,
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

      self->send(executor, init_population::value);

      return executor;
  }
};

template<typename individual, typename fitness_value, typename fitness_evaluation_operator>
class global_worker_node_driver : public worker_node_driver {
 public:
  using worker_node_driver::worker_node_driver;

  std::vector<actor> spawn_workers(stateful_actor<worker_node_executor_state> *self) override {
      auto &system = self->system();
      auto &middleman = system.middleman();
      auto &config = self->state.config;

      std::vector<actor> workers(system_props.islands_number);
      auto spawn_worker = [&] {
        auto worker = self->spawn<detached + monitored>(global_model_worker<individual,
                                                                            fitness_value,
                                                                            fitness_evaluation_operator>,
                                                        config);
        system_message(self, "Spawning worker (actor id: ", worker.id(), ")");
        return worker;
      };
      std::generate(std::begin(workers), std::end(workers), spawn_worker);

      return workers;
  }
};

/**
 * @brief This alias facilitates running global model PGA on a cluster.
 * @see cluster_runner
 */
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
    typename global_termination_check = default_global_termination_check<
        individual, fitness_value>>
using global_cluster_runner = cluster_runner<global_master_node_driver<individual,
                                                                       fitness_value,
                                                                       fitness_evaluation_operator,
                                                                       initialization_operator,
                                                                       crossover_operator,
                                                                       mutation_operator,
                                                                       parent_selection_operator,
                                                                       survival_selection_operator,
                                                                       elitism_operator,
                                                                       global_termination_check>,
                                             global_worker_node_driver<individual,
                                                                       fitness_value,
                                                                       fitness_evaluation_operator>,
                                             reporter_node_driver<individual, fitness_value>>;
}
}

#endif //GENETIC_ACTOR_GLOBAL_MODEL_CLUSTER_H

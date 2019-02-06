//
// Created by marcin on 06/01/19.
//

#ifndef GENETIC_ACTOR_GRID_MODEL_CLUSTER_H
#define GENETIC_ACTOR_GRID_MODEL_CLUSTER_H

#include <atoms.hpp>
#include <core.hpp>
#include <global_model.hpp>
#include <cluster.hpp>
#include <models/grid_model.hpp>

namespace cpga {
using namespace cluster;
namespace models {
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator>
class grid_master_node_driver : public master_node_driver {
 public:
  using master_node_driver::master_node_driver;

  actor spawn_executor(stateful_actor<base_state> *self, std::vector<actor> &workers) override {
      auto &config = self->state.config;

      auto dispatcher = self->spawn<detached>(
          grid_model_dispatcher<individual, fitness_value>,
          grid_model_dispatcher_state{config, workers});

      auto executor = self->spawn<detached + monitored>(
          grid_model_executor<individual, fitness_value, initialization_operator,
                              fitness_evaluation_operator>,
          config,
          dispatcher);

      self->send(executor, init_population::value);

      return executor;
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator>
class grid_worker_node_driver : public worker_node_driver {
 public:
  using worker_node_driver::worker_node_driver;

  std::vector<actor> spawn_workers(stateful_actor<worker_node_executor_state> *self) override {
      auto &system = self->system();
      auto &middleman = system.middleman();
      auto &state = self->state;
      auto &config = state.config;

      std::vector<actor> workers(system_props.islands_number);
      auto spawn_worker = [&]() -> actor {
        auto &worker_fun = grid_model_worker<individual, fitness_value,
                                             fitness_evaluation_operator,
                                             crossover_operator, mutation_operator,
                                             parent_selection_operator,
                                             survival_selection_operator, elitism_operator>;

        return self->template spawn<monitored + detached>(worker_fun, config);
      };
      std::generate(std::begin(workers), std::end(workers), spawn_worker);

      return workers;
  }
};

/**
 * @brief This alias facilitates running grid model PGA on a cluster.
 * @see cluster_runner
 */
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual, fitness_value>>
using grid_cluster_runner = cluster_runner<grid_master_node_driver<individual,
                                                                   fitness_value,
                                                                   fitness_evaluation_operator,
                                                                   initialization_operator>,
                                           grid_worker_node_driver<individual,
                                                                   fitness_value,
                                                                   fitness_evaluation_operator,
                                                                   crossover_operator,
                                                                   mutation_operator,
                                                                   parent_selection_operator,
                                                                   survival_selection_operator,
                                                                   elitism_operator>,
                                           reporter_node_driver<individual, fitness_value>>;
}
}

#endif //GENETIC_ACTOR_GRID_MODEL_CLUSTER_H

//
// Created by marcin on 21/01/19.
//

#ifndef GENETIC_ACTOR_GRID_MODEL_DRIVER_H
#define GENETIC_ACTOR_GRID_MODEL_DRIVER_H

#include <atoms.hpp>
#include <core.hpp>
#include <grid_model.hpp>

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>>
class grid_model_driver : public base_driver<individual, fitness_value> {
 public:
  using base_driver<individual, fitness_value>::base_driver;

  void perform(shared_config &config, actor_system &system, scoped_actor &self) override {
    auto &middleman = system.middleman();

    std::vector<actor> workers(config->system_props.islands_number);
    auto spawn_worker = [&]() -> actor {
      auto &worker_fun = grid_model_worker<individual, fitness_value,
                                           fitness_evaluation_operator,
                                           crossover_operator, mutation_operator,
                                           parent_selection_operator,
                                           survival_selection_operator, elitism_operator>;

      using worker_state = grid_model_worker_state<individual, fitness_value,
                                                   fitness_evaluation_operator,
                                                   crossover_operator, mutation_operator,
                                                   parent_selection_operator,
                                                   survival_selection_operator, elitism_operator>;

      return self->template spawn<monitored + detached>(worker_fun,
                                                        worker_state{config});
    };
    std::generate(std::begin(workers), std::end(workers), spawn_worker);

    auto dispatcher = self->spawn<detached>(
        grid_model_dispatcher<individual, fitness_value,
                              fitness_evaluation_operator, crossover_operator, mutation_operator,
                              parent_selection_operator, survival_selection_operator,
                              elitism_operator>,
        grid_model_dispatcher_state{config, workers});

    auto executor = self->spawn<detached + monitored>(
        grid_model_executor<individual, fitness_value, initialization_operator,
                            fitness_evaluation_operator>,
        grid_model_executor_state<individual, fitness_value,
                                  initialization_operator, fitness_evaluation_operator>{config},
        dispatcher);

    self->send(executor, init_population::value);
    self->wait_for(executor);
  }
};

#endif //GENETIC_ACTOR_GRID_MODEL_DRIVER_H

//
// Created by marcin on 21/01/19.
//

#ifndef GENETIC_ACTOR_GRID_MODEL_DRIVER_H
#define GENETIC_ACTOR_GRID_MODEL_DRIVER_H

#include <atoms.hpp>
#include <core.hpp>
#include <grid_model.hpp>

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator>
class grid_model_single_machine : public base_single_machine_driver<individual, fitness_value> {
 public:
  using base_single_machine_driver<individual, fitness_value>::base_single_machine_driver;

  void perform(shared_config &config, scoped_actor &self) override {
    std::vector<actor> workers(config->system_props.islands_number);
    auto spawn_worker = [&]() -> actor {
      auto &worker_fun = grid_model_worker<individual, fitness_value,
                                           fitness_evaluation_operator,
                                           crossover_operator, mutation_operator,
                                           parent_selection_operator,
                                           survival_selection_operator, elitism_operator>;
      auto worker = self->template spawn<monitored + detached>(worker_fun, config);
      system_message(self, config->system_reporter, "Spawning worker (actor id: ", worker.id(), ")");
      return worker;
    };
    std::generate(std::begin(workers), std::end(workers), spawn_worker);

    auto dispatcher = self->spawn<detached>(
        grid_model_dispatcher<individual, fitness_value>,
        grid_model_dispatcher_state{config, workers});

    auto executor = self->spawn<detached + monitored>(
        grid_model_executor<individual, fitness_value, initialization_operator,
                            fitness_evaluation_operator>,
        config,
        dispatcher);

    self->send(executor, init_population::value);
    self->wait_for(executor);
  }
};

/**
 * @brief This alias facilitates running grid model PGA on a single machine.
 * @see single_machine_runner
 */
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>>
using grid_single_machine_runner = single_machine_runner<grid_model_single_machine<individual,
                                                                                   fitness_value,
                                                                                   fitness_evaluation_operator,
                                                                                   initialization_operator,
                                                                                   crossover_operator,
                                                                                   mutation_operator,
                                                                                   parent_selection_operator,
                                                                                   survival_selection_operator,
                                                                                   elitism_operator>>;

#endif //GENETIC_ACTOR_GRID_MODEL_DRIVER_H

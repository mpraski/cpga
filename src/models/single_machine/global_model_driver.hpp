//
// Created by marcin on 21/01/19.
//

#ifndef GENETIC_ACTOR_GLOBAL_MODEL_DRIVER_H
#define GENETIC_ACTOR_GLOBAL_MODEL_DRIVER_H

#include <atoms.hpp>
#include <core.hpp>
#include <global_model.hpp>

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
class global_model_driver : public base_driver<individual, fitness_value> {
 public:
  using base_driver<individual, fitness_value>::base_driver;

  void perform(shared_config &config, actor_system &system, scoped_actor &self) override {
    auto &middleman = system.middleman();

    std::vector<actor> workers(config->system_props.islands_number);
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

    self->send(executor, init_population::value);
    self->wait_for(executor);
  }
};

#endif //GENETIC_ACTOR_GLOBAL_MODEL_DRIVER_H

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
    typename survival_selection_operator,
    typename elitism_operator,
    typename global_termination_check>
class global_model_single_machine : public base_single_machine_driver<individual, fitness_value> {
 public:
  using base_single_machine_driver<individual, fitness_value>::base_single_machine_driver;

  void perform(shared_config &config, actor_system &system, scoped_actor &self) override {
    std::vector<actor> workers(config->system_props.islands_number);
    auto spawn_worker = [&] {
      auto worker = self->spawn<detached + monitored>(global_model_worker<individual,
                                                                          fitness_value,
                                                                          fitness_evaluation_operator>,
                                                      config);
      system_message(self, config->system_reporter, "Spawning worker (actor id: ", worker.id(), ")");
      return worker;
    };
    std::generate(std::begin(workers), std::end(workers), spawn_worker);

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
    self->wait_for(executor);
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
    typename global_termination_check = default_global_termination_check<
        individual, fitness_value>>
using global_single_machine_runner = single_machine_runner<global_model_single_machine<individual,
                                                                                       fitness_value,
                                                                                       fitness_evaluation_operator,
                                                                                       initialization_operator,
                                                                                       crossover_operator,
                                                                                       mutation_operator,
                                                                                       parent_selection_operator,
                                                                                       survival_selection_operator,
                                                                                       elitism_operator,
                                                                                       global_termination_check>>;

#endif //GENETIC_ACTOR_GLOBAL_MODEL_DRIVER_H

//
// Created by marcin on 21/01/19.
//

#ifndef GENETIC_ACTOR_ISLAND_DRIVER_H
#define GENETIC_ACTOR_ISLAND_DRIVER_H

#include <atoms.hpp>
#include <core.hpp>
#include <island_model.hpp>

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator = default_survival_selection_operator<
        individual, fitness_value>,
    typename elitism_operator = default_elitism_operator<individual,
                                                         fitness_value>,
    typename migration_operator = default_migration_operator<individual,
                                                             fitness_value>>
class island_model_driver : public base_driver<individual, fitness_value> {
 public:
  using base_driver<individual, fitness_value>::base_driver;

  void perform(shared_config &config, actor_system &system, scoped_actor &self) override {
    auto &middleman = system.middleman();

    std::vector<actor> workers(config->system_props.islands_number);
    auto spawn_worker = [&] {
      return self->template spawn<monitored + detached>(island_model_worker<individual, fitness_value,
                                                                            fitness_evaluation_operator,
                                                                            initialization_operator,
                                                                            crossover_operator,
                                                                            mutation_operator,
                                                                            parent_selection_operator,
                                                                            survival_selection_operator,
                                                                            elitism_operator,
                                                                            migration_operator>, config);
    };
    std::generate(std::begin(workers), std::end(workers), spawn_worker);

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
    self->wait_for(executor);
  }
};

#endif //GENETIC_ACTOR_ISLAND_DRIVER_H

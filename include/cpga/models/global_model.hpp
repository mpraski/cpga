#pragma once

#include <algorithm>
#include <random>
#include "../core.hpp"

namespace cpga {
using namespace core;
using namespace atoms;
namespace models {
/*
 * This file defines the behaviour of global model PGA.
 * Conceptually, the Driver (global_model_single_machine) initiates the computation
 * by spawning the Executor (global_model_executor) and Supervisor (global_model_supervisor).
 * Supervisor then spawns a number of Workers (global_model_worker) which perform fitness value
 * computation when requested. The Executor carries out the main loop of computation split into
 * distinct stages (init_population, execute_state_1, ... , finish) so that parallel calculation of
 * fitness values are possible.
 *
 * Technically, each actor (Executor, Supervisor, Worker) are composed of two parts:
 *  - state (e.g. 'struct global_model_worker_state') which defines the internal state an actor can manage
 *  - behaviour (e.g. 'behavior global_model_worker(...)') which defines message handlers of an actor, for instance
 *  clause '[=](init_population) { ... }' executes its body when an actor receives the init_population message.
 */

/*
 * WORKER
 *
 * Holds a fitness_evaluation_operator in its state, uses it
 * to perform fitness value evaluation when 'compute_fitness'
 * is received.
 */
template<typename fitness_evaluation_operator>
struct global_model_worker_state : public base_state {
  global_model_worker_state() = default;
  explicit global_model_worker_state(const shared_config &config)
      : base_state{config},
        fitness_evaluation{config, island_0} {
  }

  fitness_evaluation_operator fitness_evaluation;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator>
behavior global_model_worker(stateful_actor<global_model_worker_state<fitness_evaluation_operator>> *self,
                             const shared_config &config) {
  self->state = global_model_worker_state<fitness_evaluation_operator>{config};

  return {
      [self](const individual &ind) -> fitness_value {
        return self->state.fitness_evaluation(ind);
      },
      [self](finish_worker) {
        system_message(self, "Quitting global model worker (actor id: ", self->id(), ")");
        self->quit();
      }
  };
}

/*
 * SUPERVISOR
 *
 * Spawns and manages a number of workers, its only role is to
 * delegate the 'compute_fitness' message to a uniformly chosen
 * worker.
 */
struct global_model_supervisor_state : public base_state {
  global_model_supervisor_state() = default;
  global_model_supervisor_state(const shared_config &config, std::vector<actor> &workers)
      : base_state{config},
        pool{workers.size()},
        counter{0},
        workers{std::move(workers)} {
  }

  inline auto get_worker() noexcept {
    return workers[counter++ % pool];
  }

  size_t pool;
  size_t counter;
  std::vector<actor> workers;
};

template<typename individual, typename fitness_value>
behavior global_model_supervisor(
    stateful_actor<global_model_supervisor_state> *self,
    global_model_supervisor_state state) {
  self->state = std::move(state);

  system_message(self, "Spawning global model supervisor");

  for (const auto &worker : self->state.workers) {
    self->monitor(worker);
  }

  // In the future keep reference to workers' origin nodes to remote respawn
  // them if needed
  self->set_down_handler(
      [self](down_msg &down) {
        if (!down.reason) return;

        system_message(self, "Global worker with actor id: ", down.source.id(), " died");

        auto &workers = self->state.workers;
        workers.erase(std::remove_if(std::begin(workers),
                                     std::end(workers),
                                     [src = down.source](const auto &worker) { return src == worker; }));

        self->state.pool = workers.size();
      });

  return {
      [self](individual &ind) {
        self->delegate(self->state.get_worker(), std::move(ind));
      },
      [self](finish) {
        for (const auto &worker : self->state.workers) {
          self->send(worker, finish_worker::value);
        }

        system_message(self, "Quitting global model supervisor");
        self->quit();
      }
  };
}

/*
 * EXECUTOR
 *
 * The bulk of PGA logic is implemented by the executor. It stores all the required
 * genetic operators as well as the individual and parent collections required at certain
 * stages of execution. It performs the next state of execution by sending an appropriate message
 * to itself - e.g. the 'self->send(self, execute_phase_1::value)' statement tells it to execute
 * the code defined in '[=](execute_phase_1) { ... }' after the population initialisation step is done.
 */
template<typename individual, typename fitness_value,
    typename initialization_operator, typename crossover_operator,
    typename mutation_operator, typename parent_selection_operator,
    typename global_termination_check, typename survival_selection_operator,
    typename elitism_operator>
struct global_model_executor_state : public base_state {
  global_model_executor_state() = default;
  global_model_executor_state(const shared_config &config)
      : base_state{config},
        initialization{config, island_0},
        crossover{config, island_0},
        mutation{config, island_0},
        parent_selection{config, island_0},
        survival_selection{config, island_0},
        elitism{config, island_0},
        termination_check{config, island_0},
        current_generation{0},
        current_island{0},
        compute_fitness_counter{0},
        population_size_counter{0},
        offspring_size_counter{0} {
    main.reserve(
        config->system_props.population_size
            + config->system_props.elitists_number);
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  initialization_operator initialization;
  crossover_operator crossover;
  mutation_operator mutation;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;
  global_termination_check termination_check;

  couples<individual, fitness_value> parents;
  population<individual, fitness_value> main;
  population<individual, fitness_value> offspring;
  population<individual, fitness_value> elitists;

  size_t current_generation;
  size_t current_island;
  size_t compute_fitness_counter;
  size_t population_size_counter;
  size_t offspring_size_counter;
};

template<typename individual, typename fitness_value,
    typename initialization_operator, typename crossover_operator,
    typename mutation_operator, typename parent_selection_operator,
    typename global_termination_check, typename survival_selection_operator,
    typename elitism_operator>
behavior global_model_executor(
    stateful_actor<
        global_model_executor_state<individual, fitness_value,
                                    initialization_operator, crossover_operator, mutation_operator,
                                    parent_selection_operator, global_termination_check,
                                    survival_selection_operator, elitism_operator>> *self,
    global_model_executor_state<individual, fitness_value,
                                initialization_operator, crossover_operator, mutation_operator,
                                parent_selection_operator, global_termination_check,
                                survival_selection_operator, elitism_operator> state,
    const actor &supervisor) {
  self->state = std::move(state);
  self->monitor(supervisor);

  system_message(self, "Spawning global model executor");

  self->set_down_handler([self, supervisor](down_msg &down) {
    if (down.source == supervisor) {
      system_message(self, "Quitting executor as supervisor already finished");
      self->quit();
    }
  });

  auto main_fitness_evaluation = [self, supervisor](std::function<void(decltype(self))> callback) {
    auto &state = self->state;

    state.population_size_counter = state.main.size();

    for (size_t i = 0; i < state.population_size_counter; ++i) {
      self->request(supervisor, timeout, state.main[i].first).then(
          [=](fitness_value &fv) {
            self->state.main[i].second = std::move(fv);

            if (++self->state.compute_fitness_counter == self->state.population_size_counter) {
              self->state.compute_fitness_counter = 0;
              callback(self);
            }
          },
          [=](error &err) {
            system_message(self,
                           "Phase 1: Failed to compute fitness value for individual: ",
                           self->state.offspring[i].first,
                           " with error code: ",
                           err.code());

            if (--self->state.population_size_counter == 0) {
              system_message(self, "Phase 1: Complete failure to compute fitness values, quitting...");
              self->send(self, finish::value);
            }
          }
      );
    }
  };

  auto offspring_fitness_evaluation = [self, supervisor] {
    auto &state = self->state;

    state.offspring_size_counter = state.offspring.size();

    for (size_t i = 0; i < state.offspring_size_counter; ++i) {
      self->request(supervisor, timeout, state.offspring[i].first).then(
          [=](fitness_value &fv) {
            self->state.offspring[i].second = std::move(fv);

            if (++self->state.compute_fitness_counter == self->state.offspring_size_counter) {
              self->state.compute_fitness_counter = 0;
              self->state.survival_selection(self->state.main, self->state.offspring);

              self->send(self, execute_phase_3::value);

              generation_message(self,
                                 note_end::value,
                                 now(),
                                 actor_phase::execute_phase_2,
                                 state.current_generation,
                                 state.current_island);
            }
          },
          [=](error &err) {
            system_message(self,
                           "Phase 2: Failed to compute fitness value for individual: ",
                           self->state.offspring[i].first,
                           " with error code: ",
                           err.code());

            if (--self->state.offspring_size_counter == 0) {
              system_message(self, "Phase 2: Complete failure to compute fitness values, quitting...");
              self->send(self, finish::value);
            }
          }
      );
    }
  };

  return {
      [self](init_population) {
        auto &state = self->state;

        generation_message(self, note_start::value, now(), state.current_island);
        generation_message(self, note_start::value, now(), state.current_island);

        state.initialization(std::back_inserter(state.main));
        self->send(self, execute_phase_1::value);

        generation_message(self,
                           note_end::value,
                           now(),
                           actor_phase::init_population,
                           state.current_generation,
                           state.current_island);
      },
      [self, main_fitness_evaluation](execute_phase_1) {
        generation_message(self, note_start::value, now(), self->state.current_island);

        main_fitness_evaluation([](auto self) {
          auto &state = self->state;

          self->send(self, execute_phase_2::value);

          generation_message(self,
                             note_end::value,
                             now(),
                             actor_phase::execute_phase_1,
                             state.current_generation,
                             state.current_island);
        });
      },
      [self, offspring_fitness_evaluation](execute_phase_2) {
        auto &state = self->state;
        auto &props = self->state.config->system_props;

        generation_message(self, note_start::value, now(), state.current_island);

        if (props.is_elitism_active) {
          state.elitism(state.main, state.elitists);
        }

        state.parent_selection(state.main, state.parents);

        for (const auto &couple : state.parents) {
          state.crossover(std::back_inserter(state.offspring), couple);
        }

        state.parents.clear();

        for (auto &child : state.offspring) {
          state.mutation(child);
        }

        if (props.is_survival_selection_active) {
          offspring_fitness_evaluation();
        } else {
          self->send(self, execute_phase_3::value);

          generation_message(self,
                             note_end::value,
                             now(),
                             actor_phase::execute_phase_2,
                             state.current_generation,
                             state.current_island);
        }
      },
      [self](execute_phase_3) {
        auto &state = self->state;
        auto &props = self->state.config->system_props;

        generation_message(self, note_start::value, now(), state.current_island);

        state.main.swap(state.offspring);
        state.offspring.clear();

        if (props.is_elitism_active) {
          state.main.insert(state.main.end(),
                            std::make_move_iterator(state.elitists.begin()),
                            std::make_move_iterator(state.elitists.end()));
          state.elitists.clear();
        }

        if (++state.current_generation == props.generations_number
            || state.termination_check(state.main)) {
          self->send(self, finish::value);
        } else {
          self->send(self, execute_phase_1::value);
        }

        log(self, "Generations so far: ", state.current_generation);

        generation_message(self,
                           note_end::value,
                           now(),
                           actor_phase::execute_phase_3,
                           state.current_generation,
                           state.current_island);
      },
      [self, supervisor, main_fitness_evaluation](finish) {
        main_fitness_evaluation([supervisor](auto self) {
          auto &state = self->state;

          generation_message(self,
                             note_end::value,
                             now(),
                             actor_phase::total,
                             state.current_generation,
                             state.current_island);
          individual_message(self,
                             report_population::value,
                             state.main,
                             state.current_generation,
                             state.current_island);
          self->send(supervisor, finish::value);
        });
      },
  };
}
}
}
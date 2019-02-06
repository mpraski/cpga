#pragma once

#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include "../core.hpp"

namespace cpga {
using namespace core;
using namespace atoms;
namespace models {
template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator,
    typename initialization_operator,
    typename crossover_operator,
    typename mutation_operator,
    typename parent_selection_operator,
    typename survival_selection_operator,
    typename elitism_operator,
    typename migration_operator>
struct island_model_worker_state : public base_state {
  island_model_worker_state() = default;

  island_model_worker_state(const shared_config &config, island_id id)
      : base_state{config},
        initialization{config, id},
        fitness_evaluation{config, id},
        crossover{config, id},
        mutation{config, id},
        migration{config, id},
        parent_selection{config, id},
        survival_selection{config, id},
        elitism{config, id},
        current_island{id},
        current_generation{0} {
    main.reserve(
        config->system_props.population_size
            + config->system_props.elitists_number);
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  initialization_operator initialization;
  fitness_evaluation_operator fitness_evaluation;
  crossover_operator crossover;
  mutation_operator mutation;
  migration_operator migration;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;

  island_id current_island;
  size_t current_generation;

  couples<individual, fitness_value> parents;
  population<individual, fitness_value> main;
  population<individual, fitness_value> offspring;
  population<individual, fitness_value> elitists;
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
behavior island_model_worker(
    stateful_actor<
    island_model_worker_state<individual, fitness_value,
                              fitness_evaluation_operator, initialization_operator,
                              crossover_operator, mutation_operator, parent_selection_operator,
                              survival_selection_operator, elitism_operator, migration_operator>> *self,
    const shared_config &config) {
  message_handler main_behavior{
      [self](init_population) {
        auto &state = self->state;

        generation_message(self, note_start::value, now(), state.current_island);
        generation_message(self, note_start::value, now(), state.current_island);

        state.initialization(std::back_inserter(state.main));

        generation_message(self, note_end::value, now(), actor_phase::init_population, state.current_generation,
                           state.current_island);
      },
      [self](execute_generation) {
        auto &state = self->state;
        auto &props = self->state.config->system_props;

        generation_message(self, note_start::value, now(), state.current_island);

        for (auto&[ind, value] : state.main) {
          value = state.fitness_evaluation(ind);
        }

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
          for (auto &[ind, value] : state.offspring) {
            value = state.fitness_evaluation(ind);
          }

          state.survival_selection(state.main, state.offspring);
        }

        state.main.swap(state.offspring);
        state.offspring.clear();

        if (props.is_elitism_active) {
          state.main.insert(state.main.end(),
                            std::make_move_iterator(state.elitists.begin()),
                            std::make_move_iterator(state.elitists.end()));
          state.elitists.clear();
        }

        generation_message(self, note_end::value, now(), actor_phase::execute_generation,
                           self->state.current_generation, self->state.current_island);

        ++state.current_generation;
      },
      [self](execute_migration) {
        return self->state.migration(self->state.current_island, self->state.main);
      },
      [self](receive_migration, wrapper<individual, fitness_value> &migrant) {
        self->state.main.emplace_back(std::move(migrant));
      },
      [self](finish) {
        auto &state = self->state;

        for (auto&[ind, value] : state.main) {
          value = state.fitness_evaluation(ind);
        }

        generation_message(self, note_end::value, now(), actor_phase::total, state.current_generation,
                           state.current_island);
        individual_message(self, report_population::value, state.main, state.current_generation,
                           state.current_island);
        system_message(self, "Quitting island worker id: ", state.current_island);
        bus_message(self, "worker_finished");

        self->quit();
      }
  };

  self->set_default_handler([](scheduled_actor *, message_view &) {
    return skip();
  });

  return {
      [=](assign_id, island_id id) {
        self->state = island_model_worker_state<individual, fitness_value,
                                                fitness_evaluation_operator, initialization_operator,
                                                crossover_operator, mutation_operator, parent_selection_operator,
                                                survival_selection_operator, elitism_operator, migration_operator>{
            config, id};

        self->become(main_behavior);
      }
  };
}

struct island_model_dispatcher_state : public base_state {
  island_model_dispatcher_state() = default;

  explicit island_model_dispatcher_state(const shared_config &config, std::vector<actor> &workers)
      : base_state{config},
        workers_done{0},
        migrations_done{0},
        migrations_counter{0} {
    auto gen = [id = island_id{}]() mutable {
      return id++;
    };

    for (auto &worker : workers) {
      auto id = gen();
      actor_to_island[worker.id()] = id;
      islands.emplace(id, std::move(worker));
    }
  }

  size_t workers_done;
  size_t migrations_done;
  size_t migrations_counter;
  std::unordered_map<island_id, actor> islands;
  std::unordered_map<actor_id, island_id> actor_to_island;
};

template<typename T, typename A>
inline void forward(stateful_actor<T> *self, A &&atom) {
  for (const auto &island : self->state.islands) {
    self->send(island.second, std::forward<A>(atom));
  }
}

template<typename individual, typename fitness_value>
behavior island_model_dispatcher(
    stateful_actor<island_model_dispatcher_state> *self,
    island_model_dispatcher_state state) {
  using namespace std::chrono_literals;

  self->state = std::move(state);
  self->state.join_group(*self);

  system_message(self, "Spawning island model dispatcher");

  auto islands = self->state.config->system_props.islands_number;
  for (const auto&[id, worker] : self->state.islands) {
    self->monitor(worker);
    self->send(worker, assign_id::value, id);
  }

  /*
   * Monitor island workers, restart ones that die
   * for abnormal reasons
   */
  self->set_down_handler([self](down_msg &down) {
    if (!down.reason) return;

    auto &state = self->state;
    auto id = state.actor_to_island[down.source.id()];

    system_message(self, "Island worker with id: ", id, " died");

    state.islands.erase(id);
    state.actor_to_island.erase(down.source.id());
  });

  return {
      /*
       * Short handlers below only forward the given
       * message to the every island worker
       */
      [self](init_population atom) {
        forward(self, atom);
      },
      [self](execute_generation atom) {
        forward(self, atom);
      },
      /*
       * Run the migration step, that is request migrants from islands and
       * route them to their destinations, then notify the executor by
       * delivering the response promise.
       */
      [self, islands](execute_migration atom) -> result<bool> {
        self->state.migrations_counter = islands;

        auto rp = self->make_response_promise<bool>();
        for (const auto&[id, worker] : self->state.islands) {
          self->request(worker, infinite, atom).then(
              [=](const migration_payload<individual, fitness_value> &payload) mutable {
                auto &islands = self->state.islands;

                for (auto&[island_id, migrant] : payload) {
                  if (auto island{islands.find(island_id)}; island != islands.end()) {
                    self->send(island->second, receive_migration::value, std::move(migrant));
                  }
                }

                if (++self->state.migrations_done == self->state.migrations_counter) {
                  self->state.migrations_done = 0;
                  rp.deliver(true);
                }
              },
              [=](error &err) mutable {
                system_message(self, "Failed to execute migration for island: ", id,
                               " with error code: ", err.code());

                if (--self->state.migrations_counter == 0) {
                  system_message(self, "Complete failure to perform migration, quitting...");
                  rp.deliver(false);
                  self->send(self, finish::value);
                }
              }
          );
        }

        return rp;
      },
      [self](finish atom) {
        forward(self, atom);
      },
      /*
       * Receive 'end-of-work' signals from workers and
       * terminate when all are done
       */
      bus_receive("worker_finished", [self, islands](const std::string &) {
        if (++self->state.workers_done == islands) {
          system_message(self, "Quitting dispatcher as all ", islands, " island workers are done");
          self->quit();
        }
      }),
  };
}

struct island_model_executor_state : public base_state {
  island_model_executor_state() = default;

  explicit island_model_executor_state(const shared_config &config)
      : base_state{config},
        generations_so_far{0} {
  }

  size_t generations_so_far;
};

behavior island_model_executor(
    stateful_actor<island_model_executor_state> *self,
    island_model_executor_state state,
    const actor &dispatcher) {
  self->state = std::move(state);
  self->monitor(dispatcher);

  system_message(self, "Spawning island model executor");

  self->set_down_handler([self, dispatcher](down_msg &down) {
    if (down.source == dispatcher) {
      system_message(self, "Quitting executor as dispatcher already finished");
      self->quit();
    }
  });

  const auto &props = self->state.config->system_props;

  return {
      [=](execute_phase_1) {
        self->send(dispatcher, init_population::value);

        if (props.is_migration_active) {
          self->send(self, execute_phase_2::value, props.migration_period);
        } else {
          self->send(self, execute_phase_3::value);
        }
      },
      [=](execute_phase_2, size_t period) {
        for (size_t i = 0; i < period; ++i) {
          self->send(dispatcher, execute_generation::value);
        }

        self->request(dispatcher, infinite, execute_migration::value).await(
            [&](bool flag) {
              if (!flag) return;
              auto &props = self->state.config->system_props;

              self->state.generations_so_far += period;

              log(self, "Generations so far: ", self->state.generations_so_far);

              if (self->state.generations_so_far < props.generations_number) {
                self->send(self,
                           execute_phase_2::value,
                           std::min(props.migration_period, props.generations_number - self->state.generations_so_far));
              } else {
                self->send(self, execute_phase_4::value);
              }
            }
        );
      },
      [=](execute_phase_3) {
        for (size_t i = 0; i < props.generations_number; ++i) {
          self->send(dispatcher, execute_generation::value);
          log(self, "Generations so far: ", i + 1);
        }
        self->send(self, execute_phase_4::value);
      },
      [=](execute_phase_4) {
        self->send(dispatcher, finish::value);
      },
  };
}
}
}
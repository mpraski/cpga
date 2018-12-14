#pragma once

#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include "../core.hpp"

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator, typename migration_operator>
struct island_model_worker_state : public base_state {
  island_model_worker_state() = default;
  island_model_worker_state(const shared_config& config, const island_id id)
      : base_state { config },
        initialization { config, id },
        fitness_evaluation { config, id },
        crossover { config, id },
        mutation { config, id },
        migration { config, id },
        parent_selection { config, id },
        survival_selection { config, id },
        elitism { config, id },
        current_island { id },
        current_generation { 0 } {
    population.reserve(
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
  std::size_t current_generation;

  parent_collection<individual, fitness_value> parents;
  individual_collection<individual, fitness_value> population;
  individual_collection<individual, fitness_value> offspring;
  individual_collection<individual, fitness_value> elitists;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator, typename migration_operator>
behavior island_model_worker(
    stateful_actor<
        island_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, parent_selection_operator,
            survival_selection_operator, elitism_operator, migration_operator>>* self,
    island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, parent_selection_operator,
        survival_selection_operator, elitism_operator, migration_operator> state,
    const actor& dispatcher) {
  self->state = std::move(state);

  return {
    [self](init_population) {
      auto& state = self->state;

      generation_message(self, note_start::value, now(), state.current_island);
      generation_message(self, note_start::value, now(), state.current_island);

      state.initialization(state.population);

      generation_message(self, note_end::value, now(), actor_phase::init_population, state.current_generation, state.current_island);
    },
    [self](execute_generation) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      generation_message(self, note_start::value, now(), state.current_island);

      for (auto& member : state.population) {
        member.second = state.fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        state.elitism(state.population, state.elitists);
      }

      state.parent_selection(state.population, state.parents);

      for (const auto& parent : state.parents) {
        state.crossover(state.offspring, parent);
      }

      state.parents.clear();

      for (auto& child : state.offspring) {
        state.mutation(child);
      }

      if (props.is_survival_selection_active) {
        for (auto& child : state.offspring) {
          child.second = state.fitness_evaluation(child.first);
        }

        state.survival_selection(state.population, state.offspring);
      }

      state.population.swap(state.offspring);
      state.offspring.clear();

      if (props.is_elitism_active) {
        state.population.insert(state.population.end(),
            std::make_move_iterator(state.elitists.begin()),
            std::make_move_iterator(state.elitists.end()));
        state.elitists.clear();
      }

      generation_message(self, note_end::value, now(), actor_phase::execute_generation, self->state.current_generation, self->state.current_island);

      ++state.current_generation;
    },
    [self](execute_migration) {
      return self->state.migration(self->state.current_island, self->state.population);
    },
    [self](receive_migration, individual_wrapper<individual, fitness_value> migrant) {
      self->state.population.emplace_back(std::move(migrant));
    },
    [self, dispatcher](finish) {
      auto& state = self->state;

      generation_message(self, note_end::value, now(), actor_phase::total, state.current_generation, state.current_island);
      individual_message(self, report_population::value, state.population, state.current_generation, state.current_island);
      system_message(self, "Quitting island worker id: ", state.current_island);

      self->send(dispatcher, worker_finished::value);
      self->quit();
    }
  };
}

struct island_model_dispatcher_state : public base_state {
  island_model_dispatcher_state() = default;
  island_model_dispatcher_state(const shared_config& config)
      : base_state { config },
        workers_done { 0 },
        migrations_done { 0 },
        migrations_counter { 0 } {
  }

  std::size_t workers_done;
  std::size_t migrations_done;
  std::size_t migrations_counter;
  std::unordered_map<island_id, actor> islands;
  std::unordered_map<actor_id, island_id> actor_to_island;
};

template<typename T, typename A>
inline void forward(stateful_actor<T>* self, A&& atom) {
  for (const auto& x : self->state.islands) {
    self->send(x.second, std::forward<A>(atom));
  }
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator, typename migration_operator>
behavior island_model_dispatcher(
    stateful_actor<island_model_dispatcher_state>* self,
    island_model_dispatcher_state state) {
  self->state = std::move(state);

  system_message(self, "Spawning island model dispatcher");

  auto islands = self->state.config->system_props.islands_number;

  auto spawn_worker = [self](island_id id) -> actor {
    const auto& state = self->state;
    const auto& config = state.config;

    auto& worker_fun = island_model_worker<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator, migration_operator>;

    using worker_state = island_model_worker_state<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator, migration_operator>;

    return self->template spawn<monitored + detached>(worker_fun,
        worker_state {config, id}, self);
  };

  /*
   * Islands are given id in range [0, islands_number-1]
   * An id is guaranteed to stay valid throughout the execution,
   * although the island it points to might be restarted as a new actor
   * if it goes down.
   */
  for (island_id i = 0; i < islands; ++i) {
    auto island = spawn_worker(i);

    system_message(self, "Spawning new island worker with id: ", i,
                   " (actor id: ", island.id(), ")");

    self->state.actor_to_island[island.id()] = i;
    self->state.islands.emplace(i, std::move(island));
  }

  /*
   * Monitor island workers, restart ones that die
   * for abnormal reasons
   */
  self->set_down_handler([self, spawn_worker](down_msg& down) {
    if(!down.reason) return;

    auto& state = self->state;
    auto id = state.actor_to_island[down.source.id()];
    auto island = spawn_worker(id);
    auto island_actor_id = island.id();

    system_message(self, "Island worker with id: ", id, " died, respawning..");

    state.islands.erase(id);
    state.islands.emplace(id, std::move(island));
    state.actor_to_island.erase(down.source.id());
    state.actor_to_island[island_actor_id] = id;
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
     * delivering the reponse promise.
     */
    [self, islands](execute_migration atom) -> result<bool> {
      self->state.migrations_counter = islands;

      auto rp = self->make_response_promise<bool>();
      for (const auto& x : self->state.islands) {
        self->request(x.second, timeout, atom).then(
            [=](const migration_payload<individual, fitness_value>& payload) mutable {
              for (const auto& pair : payload) {
                auto& island = self->state.islands[pair.first];
                auto& migrant = pair.second;

                self->send(island, receive_migration::value, migrant);
              }

              if(++self->state.migrations_done == self->state.migrations_counter) {
                self->state.migrations_done = 0;
                rp.deliver(true);
              }
            },
            [=](error& err) {
              system_message(self, "Failed to execute migration for island: ", x.first, " with error code: ", err.code());

              if(--self->state.migrations_counter == 0) {
                system_message(self, "Complete failure to perform migration, quitting...");
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
    [self, islands](worker_finished) {
      if(++self->state.workers_done == islands) {
        system_message(self, "Quitting dispatcher as all ", islands, " island workers are done");
        self->quit();
      }
    }
  };
}

struct island_model_executor_state : public base_state {
  island_model_executor_state() = default;
  island_model_executor_state(const shared_config& config)
      : base_state { config },
        generations_so_far { 0 } {
  }

  std::size_t generations_so_far;
};

behavior island_model_executor(
    stateful_actor<island_model_executor_state>* self,
    island_model_executor_state state, const actor& dispatcher) {
  self->state = std::move(state);
  self->monitor(dispatcher);

  system_message(self, "Spawning island model executor");

  self->set_down_handler([self, dispatcher](down_msg& down) {  // @suppress("Invalid arguments")
        if(down.source == dispatcher) {
          system_message(self, "Quitting executor as dispatcher already finished");
          self->quit();
        }
      });

  const auto& props = self->state.config->system_props;

  return {
    [=](execute_phase_1) {
      self->send(dispatcher, init_population::value);

      if(props.is_migration_active) {
        self->send(self, execute_phase_2::value);
      } else {
        self->send(self, execute_phase_3::value);
      }
    },
    [=](execute_phase_2) {
      for (std::size_t i = 0; i < props.migration_period; ++i) {
        self->send(dispatcher, execute_generation::value);
      }

      self->request(dispatcher, infinite, execute_migration::value).await([=](bool flag) {
            self->state.generations_so_far += props.migration_period;

            if(self->state.generations_so_far + props.migration_period <= props.generations_number) {
              self->send(self, execute_phase_2::value);
            } else {
              self->send(self, execute_phase_4::value);
            }
          }
      );
    },
    [=](execute_phase_3) {
      for (std::size_t i = 0; i < props.generations_number; ++i) {
        self->send(dispatcher, execute_generation::value);
      }
      self->send(self, execute_phase_4::value);
    },
    [=](execute_phase_4) {
      self->send(dispatcher, finish::value);
    },
  };
}

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
class island_model_driver : private base_driver {
 public:
  using base_driver::base_driver;

  void run() {
    actor_system_config cfg;
    actor_system system { cfg };
    scoped_actor self { system };
    configuration* conf { new configuration { system_props, user_props } };
    shared_config config { conf };

    start_reporters<individual, fitness_value>(*conf, system, self);

    auto& dispatcher_func = island_model_dispatcher<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, parent_selection_operator,
        survival_selection_operator, elitism_operator, migration_operator>;

    auto dispatcher = system.spawn(dispatcher_func,
                                   island_model_dispatcher_state { config });

    auto executor = system.spawn(island_model_executor,
                                 island_model_executor_state { config },
                                 dispatcher);

    self->send(executor, execute_phase_1::value);
    self->wait_for(executor);

    stop_reporters(*conf, self);
  }
};

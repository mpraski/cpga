#pragma once

#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include "../core.hpp"

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
struct island_model_worker_state : public base_state {
  island_model_worker_state() = default;

  island_model_worker_state(const shared_config& config, const island_id id,
                            const initialization_operator& io,
                            const fitness_evaluation_operator& feo,
                            const crossover_operator& co,
                            const mutation_operator& mo,
                            const migration_operator& muo,
                            const parent_selection_operator& pso,
                            const survival_selection_operator& sso,
                            const elitism_operator& eo)
      : base_state { config },
        initialization { io },
        fitness_evaluation { feo },
        crossover { co },
        mutation { mo },
        migration { muo },
        parent_selection { pso },
        survival_selection { sso },
        elitism { eo },
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
    typename migration_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
behavior island_model_worker(
    stateful_actor<
        island_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator, survival_selection_operator,
            elitism_operator>>* self,
    island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, migration_operator,
        parent_selection_operator, survival_selection_operator, elitism_operator> state,
    const actor& dispatcher) {
  self->state = std::move(state);

  return {
    [self](init_population) {
      auto& state = self->state;

      state.initialization(state.population);
    },
    [self](execute_generation) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

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

      ++state.current_generation;
    },
    [self, dispatcher](migrate_request) {
      auto& state = self->state;
      self->send(dispatcher, migrate::value, state.migration(state.current_island, state.population));
    },
    [self](migrate_receive, individual_wrapper<individual, fitness_value> migrant) {
      self->state.population.emplace_back(std::move(migrant));
    },
    [self, dispatcher](finish) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_individual_reporter_active) {
        auto& individual_reporter = state.config->individual_reporter;

        self->send(individual_reporter, report_population::value, state.population, state.current_generation, state.current_island);
      }

      system_message(self, "Quitting island worker id: ", state.current_island);

      self->send(dispatcher, worker_finished::value);
      self->quit();
    }
  };
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
struct island_model_dispatcher_state : public base_state {
  island_model_dispatcher_state() = default;

  island_model_dispatcher_state(const shared_config& config,
                                fitness_evaluation_operator feo,
                                initialization_operator io,
                                crossover_operator co, mutation_operator mo,
                                migration_operator muo,
                                parent_selection_operator pso,
                                survival_selection_operator sso,
                                elitism_operator eo)
      : base_state { config },
        workers_done { 0 },
        fitness_evaluation { std::move(feo) },
        initialization { std::move(io) },
        crossover { std::move(co) },
        mutation { std::move(mo) },
        migration { std::move(muo) },
        parent_selection { std::move(pso) },
        survival_selection { std::move(sso) },
        elitism { std::move(eo) } {
  }

  std::size_t workers_done;
  std::unordered_map<island_id, actor> islands;
  std::unordered_map<actor_id, island_id> actor_to_island;

  fitness_evaluation_operator fitness_evaluation;
  initialization_operator initialization;
  crossover_operator crossover;
  mutation_operator mutation;
  migration_operator migration;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;
};

template<typename T, typename A>
inline void forward(stateful_actor<T>* self, A&& atom) {
  for (const auto& x : self->state.islands) {
    self->delegate(x.second, std::forward<A>(atom));
  }
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
behavior island_model_dispatcher(
    stateful_actor<
        island_model_dispatcher_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator, survival_selection_operator,
            elitism_operator>>* self,
    island_model_dispatcher_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, migration_operator,
        parent_selection_operator, survival_selection_operator, elitism_operator> state) {
  self->state = std::move(state);

  system_message(self, "Spawning island model dispatcher");

  auto islands = self->state.config->system_props.islands_number;

  auto spawn_worker = [self](island_id id) -> actor {
    const auto& state = self->state;
    const auto& config = state.config;
    const auto& io = state.initialization;
    const auto& feo = state.fitness_evaluation;
    const auto& co = state.crossover;
    const auto& mo = state.mutation;
    const auto& muo = state.migration;
    const auto& pso = state.parent_selection;
    const auto& sso = state.survival_selection;
    const auto& eo = state.elitism;

    auto worker_fun = island_model_worker<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator, migration_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator>;

    using worker_state = island_model_worker_state<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator, migration_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator>;

    return self->template spawn<monitored + detached>(worker_fun,
        worker_state {config, id, io, feo, co, mo, muo, pso, sso, eo}, self);
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

    state.islands[id] = std::move(island);
    state.actor_to_island.erase(down.source.id());
    state.actor_to_island.emplace(island_actor_id, id);
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
    [self](migrate_request atom) {
      forward(self, atom);
    },
    [self](finish atom) {
      forward(self, atom);
    },
    /*
     * Upon receiving a migration payload from an island, send
     * each migrant to appropriate island
     */
    [self](migrate, const migration_payload<individual, fitness_value>& payload) {
      for (const auto& pair : payload) {
        auto& island = self->state.islands[pair.first];
        auto& migrant = pair.second;

        self->send(island, migrate_receive::value, migrant);
      }
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

template<typename individual, typename fitness_value>
struct island_model_executor_state : public base_state {

};

template<typename individual, typename fitness_value>
behavior island_model_executor(
    stateful_actor<island_model_executor_state<individual, fitness_value>>* self,
    island_model_executor_state<individual, fitness_value> state,
    const actor& dispatcher) {
  self->state = std::move(state);
  self->monitor(dispatcher);

  system_message(self, "Spawning island model executor");

  self->set_down_handler([self, dispatcher](down_msg& down) {
    if(down.source.id() == dispatcher.id()) {
      system_message(self, "Quitting executor as dispatcher already finished");

      self->quit();
    }
  });

  return {
    [=](execute_phase_1) {
      self->send(dispatcher, init_population::value);
      self->send(self, execute_phase_2::value);
    },
    [=](execute_phase_2) {
      auto& props = self->state.config->system_props;
      for (std::size_t i = 0; i < props.generations_number; ++i) {
        self->send(dispatcher, execute_generation::value);
      }
      self->send(self, execute_phase_3::value);
    },
    [=](execute_phase_3) {
      self->send(dispatcher, finish::value);
    },
  };
}

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename survival_selection_operator, typename elitism_operator>
class island_model_driver : private base_driver {
 public:
  using base_driver::base_driver;

  void run() {
    actor_system_config cfg;
    actor_system system { cfg };
    scoped_actor self { system };
    configuration* conf { new configuration { system_props, user_props } };

    start_reporters<individual, fitness_value>(*conf, system, self);

    shared_config config { conf };
    fitness_evaluation_operator fitness_evaluation { config };
    initialization_operator initialization { config };
    crossover_operator crossover { config };
    mutation_operator mutation { config };
    migration_operator migration { config };
    parent_selection_operator parent_selection { config };
    survival_selection_operator survival_selection { config };
    elitism_operator elitism { config };

    auto dispatcher = system.spawn(
        island_model_dispatcher<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator, survival_selection_operator,
            elitism_operator>,
        island_model_dispatcher_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator, survival_selection_operator,
            elitism_operator> { config, std::move(fitness_evaluation),
            std::move(initialization), std::move(crossover), std::move(
                mutation), std::move(migration), std::move(parent_selection),
            std::move(survival_selection), std::move(elitism) });

    auto executor = system.spawn(
        island_model_executor<individual, fitness_value>,
        island_model_executor_state<individual, fitness_value> { config },
        dispatcher);

    self->send(executor, execute_phase_1::value);

    self->wait_for(executor);

    stop_reporters(*conf, self);
  }
};

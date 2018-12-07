#pragma once

#include <algorithm>
#include <random>
#include "../core.hpp"

using namespace caf;

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
        current_island { id },
        initialization { io },
        fitness_evaluation { feo },
        crossover { co },
        mutation { mo },
        migration { muo },
        parent_selection { pso },
        survival_selection { sso },
        elitism { eo },
        current_generation { 0 } {
    population.reserve(
        config->system_props.population_size
            + config->system_props.elitists_number);
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  island_id current_island;

  initialization_operator initialization;
  fitness_evaluation_operator fitness_evaluation;
  crossover_operator crossover;
  mutation_operator mutation;
  migration_operator migration;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;

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
  self->join(self->system().groups().get_local("islands"));

  return {
    [self](init_population) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_start::value, now());
        self->send(actor_reporter, note_start::value, now());
      }

      state.initialization(state.population);

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_end::value, now());
        self->send(actor_reporter, report_info::value, actor_phase::init_population, state.current_generation, state.current_island);
      }
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

      // This will fill parents with individual_wrapper_pairs, each holding two copied individuals
      state.parent_selection(state.population, state.parents);

      // This will fill offspring with newly created individual_wrappers
      for (const auto& parent : state.parents) {
        state.crossover(state.offspring, parent);
      }

      // Clear parents for future use
      state.parents.clear();

      // This will apply mutation to each child in offspring
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
    [self](finish) {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      if(props.is_actor_reporter_active) {
        auto& actor_reporter = state.config->actor_reporter;

        self->send(actor_reporter, note_end::value, now());
        self->send(actor_reporter, report_info::value, actor_phase::total, state.current_generation, state.current_island);
      }

      if(props.is_individual_reporter_active) {
        auto& individual_reporter = state.config->individual_reporter;

        self->send(individual_reporter, report_population::value, state.population, state.current_generation, state.current_island);
      }

      //self->quit();
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
                                initialization_operator io,
                                fitness_evaluation_operator feo,
                                crossover_operator co, mutation_operator mo,
                                migration_operator muo,
                                parent_selection_operator pso,
                                survival_selection_operator sso,
                                elitism_operator eo)
      : base_state { config },
        initialization { std::move(io) },
        fitness_evaluation { std::move(feo) },
        crossover { std::move(co) },
        mutation { std::move(mo) },
        migration { std::move(muo) },
        parent_selection { std::move(pso) },
        survival_selection { std::move(sso) },
        elitism { std::move(eo) } {
  }

  std::unordered_map<island_id, actor> islands;
  std::unordered_map<actor_id, island_id> actor_to_island;

  group islands_group;

  initialization_operator initialization;
  fitness_evaluation_operator fitness_evaluation;
  crossover_operator crossover;
  mutation_operator mutation;
  migration_operator migration;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;
};

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
  self->state.islands_group = self->system().groups().get_local("islands");

  auto spawn_worker =
      [self](island_id id) -> actor {
        auto& state = self->state;
        auto& config = state.config;
        auto& io = state.initialization;
        auto& feo = state.fitness_evaluation;
        auto& co = state.crossover;
        auto& mo = state.mutation;
        auto& muo = state.migration;
        auto& pso = state.parent_selection;
        auto& sso = state.survival_selection;
        auto& eo = state.elitism;

        auto worker = self->spawn(
            island_model_worker<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator,
            survival_selection_operator, elitism_operator>,
            island_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator,
            survival_selection_operator, elitism_operator> {config, id, io, feo, co, mo, muo, pso, sso, eo}, self);

        self->monitor(worker);

        return worker;
      };

  // Islands are given id in range [0, islands_number-1]
  // An id is guaranteed to stay valid throughout the execution,
  // although the island it points to might be restarted as a new actor
  // if it goes down.
  for (island_id i = 0; i < self->state.config->system_props.islands_number;
      ++i) {
    auto island = spawn_worker(i);

    self->state.actor_to_island.emplace(island.id(), i);
    self->state.islands.emplace(i, std::move(island));
  }

  return {
    /*
     * Short handlers below only forward the given
     * message to the every island worker
     */
    [self](init_population atom) {
      self->send(self->state.islands_group, atom);
    },
    [self](execute_generation atom) {
      self->send(self->state.islands_group, atom);
    },
    [self](migrate_request atom) {
      self->send(self->state.islands_group, atom);
    },
    [self](finish atom) {
      self->send(self->state.islands_group, atom);
      //self->quit();
    },
    /*
     * Upon receiving a migration payload from an island, send
     * each migrant to appropriate island
     */
    [self](migrate, const migration_payload<individual, fitness_value>& payload) {
      // Send migrants to respective islands as they arrive
      for (const auto& pair : payload) {
        auto& island = self->state.islands[pair.first];
        auto& migrant = pair.second;

        self->send(island, migrate_receive::value, migrant);
      }
    },
    /*
     * Handle the death of an island worker actor
     */
    [self, spawn_worker](const down_msg& down) {
      system_message(self, "Island worker with id: ", down.source.id(), " died, respawning..");

      auto& state = self->state;
      auto id = state.actor_to_island[down.source.id()];
      auto island = spawn_worker(id);
      auto island_actor_id = island.id();

      state.islands[id] = std::move(island);
      state.actor_to_island.erase(down.source.id());
      state.actor_to_island.emplace(island_actor_id, id);
    },
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

  return {

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
            elitism_operator> { config, std::move(initialization), std::move(
            fitness_evaluation), std::move(crossover), std::move(mutation),
            std::move(migration), std::move(parent_selection), std::move(
                survival_selection), std::move(elitism) });

    self->send(dispatcher, init_population::value);
    self->send(dispatcher, execute_generation::value);
    self->send(dispatcher, finish::value);

    self->wait_for(dispatcher);

    // Quit reporters and supervisor
    stop_reporters(*conf, self);
    anon_send_exit(dispatcher, exit_reason::user_shutdown);
  }
};

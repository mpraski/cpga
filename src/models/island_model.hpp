#pragma once

#include <algorithm>
#include <random>
#include "../core.hpp"

using namespace caf;

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename global_temination_check, typename survival_selection_operator,
    typename elitism_operator>
struct island_model_worker_state : public base_state {
  island_model_worker_state() = default;

  island_model_worker_state(const shared_config& config, island_id id)
      : base_state { config },
        current_island { id } {
  }

  parent_collection<individual, fitness_value> parents;
  individual_collection<individual, fitness_value> population;
  individual_collection<individual, fitness_value> offspring;
  individual_collection<individual, fitness_value> elitists;

  initialization_operator initialization;
  crossover_operator crossover;
  mutation_operator mutation;
  migration_operator migration;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;
  global_temination_check termination_check;

  island_id current_island;
  std::size_t current_generation;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename global_temination_check, typename survival_selection_operator,
    typename elitism_operator>
behavior island_model_worker(
    stateful_actor<
        island_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, migration_operator,
            parent_selection_operator, global_temination_check,
            survival_selection_operator, elitism_operator>>* self,
    island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, migration_operator,
        parent_selection_operator, global_temination_check,
        survival_selection_operator, elitism_operator> state,
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
        member.second = fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        state.elitism(state.population, state.elitists);
      }

      // This will fill parents with individual_wrapper_pairs, each holding two copied individuals
      state.parent_selection(state.population, state.parents);

      // This will fill offspring with newly created individual_wrappers
      for (const auto& parent : state.parents) {
        crossover(state.offspring, parent);
      }

      // Clear parents for future use
      state.parents.clear();

      // This will apply mutation to each child in offspring
      for (auto& child : state.offspring) {
        mutation(child);
      }

      if (props.is_survival_selection_active) {
        for (auto& child : state.offspring) {
          child.second = fitness_evaluation(child.first);
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
      self->send(dispatcher, migrate::value, state.migration(state->id, state->population));
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
    }
  };
}

template<typename individual, typename fitness_value>
struct island_model_dispatcher_state : public base_state {
  std::unordered_map<island_id, actor> islands;
  std::unordered_map<actor_id, island_id> actor_to_island;
  group islands_group;

  island_model_dispatcher_state() = default;

  island_model_dispatcher_state(const shared_config& config)
      : base_state { config } {
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename migration_operator, typename parent_selection_operator,
    typename global_temination_check, typename survival_selection_operator,
    typename elitism_operator>
behavior island_model_dispatcher(
    stateful_actor<island_model_dispatcher_state<individual, fitness_value>>* self,
    island_model_dispatcher_state<individual, fitness_value> state) {
  self->state = std::move(state);
  self->state.islands_group = self->system().groups().get_local("islands");

  auto spawn_worker = [self](island_id id) -> actor {
    return self->spawn<monitored + detached>(
        island_model_worker,
        island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, migration_operator,
        parent_selection_operator, global_temination_check,
        survival_selection_operator, elitism_operator> {self->state
          .config, id}, self);
  };

  // Islands are given id in range [0, islands_number-1]
  // An id is guaranteed to stay valid throughout the execution,
  // although the island it points to might be restarted as a new actor
  // if it goes down.
  for (island_id i = 0; i < self->state.config->islands_number; ++i) {
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
    island_model_executor_state<individual, fitness_value> state) {
  self->state = std::move(state);

  return {

  };
}

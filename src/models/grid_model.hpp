#pragma once

#include "../core.hpp"

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator>
struct grid_model_worker_state : public base_state {
  grid_model_worker_state() = default;
  grid_model_worker_state(const shared_config& config, const std::size_t id)
      : base_state { config },
        initialization { config, id },
        fitness_evaluation { config, id },
        crossover { config, id },
        mutation { config, id },
        parent_selection { config, id },
        survival_selection { config, id },
        elitism { config, id },
        current_island { id },
        current_generation { 0 } {
    offspring.reserve(config->system_props.population_size);
    elitists.reserve(config->system_props.elitists_number);
  }

  initialization_operator initialization;
  fitness_evaluation_operator fitness_evaluation;
  crossover_operator crossover;
  mutation_operator mutation;
  parent_selection_operator parent_selection;
  survival_selection_operator survival_selection;
  elitism_operator elitism;

  std::size_t current_island;
  std::size_t current_generation;

  parent_collection<individual, fitness_value> parents;
  individual_collection<individual, fitness_value> offspring;
  individual_collection<individual, fitness_value> elitists;

  inline void reset() {
    parents.clear();
    offspring.clear();
    elitists.clear();
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator>
behavior grid_model_worker(
    stateful_actor<
        grid_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, parent_selection_operator,
            survival_selection_operator, elitism_operator>>* self,
    grid_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, parent_selection_operator,
        survival_selection_operator, elitism_operator> state) {
  self->state = std::move(state);

  return {
    [self](individual_collection<individual, fitness_value> pop) -> individual_collection<individual, fitness_value> {
      auto& state = self->state;
      auto& props = self->state.config->system_props;

      auto population = std::move(pop);
      state.clear();

      generation_message(self, note_start::value, now(), state.current_island);

      for (auto& member : population) {
        member.second = state.fitness_evaluation(member.first);
      }

      if (props.is_elitism_active) {
        state.elitism(population, state.elitists);
      }

      state.parent_selection(population, state.parents);

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

        state.survival_selection(population, state.offspring);
      }

      population.swap(state.offspring);

      if (props.is_elitism_active) {
        population.insert(population.end(),
            std::make_move_iterator(state.elitists.begin()),
            std::make_move_iterator(state.elitists.end()));
        state.elitists.clear();
      }

      generation_message(self, note_end::value, now(), actor_phase::execute_generation, self->state.current_generation, self->state.current_island);

      ++state.current_generation;

      return population;
    },
    [self](finish_worker) {
      system_message(self, "Quitting global model worker (actor id: ", self->id(), ")");
      self->quit();
    }
  };
}

struct grid_model_dispatcher_state : public base_state {
  grid_model_dispatcher_state() = default;
  grid_model_dispatcher_state(const shared_config& config,
                              std::size_t pool_size)
      : base_state { config },
        pool_size { pool_size } {
    workers.reserve(pool_size);
  }

  std::size_t pool_size;
  std::vector<actor> workers;
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename survival_selection_operator,
    typename elitism_operator>
behavior grid_model_dispatcher(
    stateful_actor<grid_model_dispatcher_state>* self,
    grid_model_dispatcher_state state) {
  self->state = std::move(state);

  system_message(self, "Spawning grid model dispatcher");

  auto spawn_worker = [self](std::size_t id) -> actor {
    const auto& state = self->state;
    const auto& config = state.config;

    auto& worker_fun = grid_model_worker<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator>;

    using worker_state = grid_model_worker_state<individual, fitness_value,
    fitness_evaluation_operator, initialization_operator,
    crossover_operator, mutation_operator,
    parent_selection_operator,
    survival_selection_operator, elitism_operator>;

    return self->template spawn<monitored + detached>(worker_fun,
        worker_state {config, id});
  };

  for (std::size_t i = 0; i < self->state.pool_size; ++i) {
    auto worker = spawn_worker(i);

    system_message(self, "Spawning new grid worker with actor id: ",
                   worker.id());

    self->state.workers.emplace_back(std::move(worker));
  }

  self->set_down_handler(
      [self, spawn_worker](down_msg& down) {
        if(!down.reason) return;

        system_message(self, "Grid worker with actor id: ", down.source.id(), " died, respawning...");

        auto& workers = self->state.workers;
        auto it = workers.begin();

        for(;it != workers.end(); ++it) {
          if(*it == down.source) break;
        }

        workers.erase(it);
        workers.emplace_back(spawn_worker());
      });

  return {
    [self](execute_computation atom, std::size_t id, individual_collection<individual, fitness_value> pop) {
      self->delegate(self->state.workers[id], atom, std::move(pop));
    },
    [self](finish) {
      for(const auto& worker : self->state.workers) {
        self->send(worker, finish_worker::value);
      }

      system_message(self, "Quitting grid model dispatcher");
      self->quit();
    }
  };
}

template<typename individual, typename fitness_value>
struct grid_model_executor_state : public base_state {
  grid_model_executor_state() = default;
  grid_model_executor_state(const shared_config& config)
      : base_state { config } {
    population.reserve(config->system_props.population_size);
    result.reserve(config->system_props.population_size);
  }

  individual_collection<individual, fitness_value> population;
  individual_collection<individual, fitness_value> result;
};

template<typename individual, typename fitness_value>
behavior grid_model_executor(
    stateful_actor<grid_model_executor_state<individual, fitness_value>>* self,
    grid_model_executor_state<individual, fitness_value> state,
    const actor& dispatcher) {
  self->state = std::move(state);
  self->monitor(dispatcher);

  system_message(self, "Spawning grid model executor");

  self->set_down_handler([self, dispatcher](down_msg& down) {  // @suppress("Invalid arguments")
        if(down.source == dispatcher) {
          system_message(self, "Quitting executor as dispatcher already finished");
          self->quit();
        }
      });

  const auto& props = self->state.config->system_props;

  return {
    [=](execute_phase_1) {

    },
    [=](execute_phase_2) {
      for (island_id i = 0; i < props.islands_number; ++i) {
        self->send(dispatcher, execute_generation::value);
      }

      auto times = props.population_size / props.islands_number;
      auto rem = props.population_size % props.islands_number;
      auto random_nums = shuffled(props.population_size);
      std::size_t c = 0;

      for(island_id i = 0; i < props.islands_number; ++i) {
        individual_collection<individual, fitness_value> pop(times);
        for(std::size_t j = 0; j < times; ++j) {
          pop[j] = self->state.population[random_nums[c++]];
        }

        self->request(dispatcher, timeout, pop,
            [=](individual_collection<individual, fitness_value>& result) {
              self->state.population.insert(self->state.population.end(),
                  std::make_move_iterator(result.begin()),
                  std::make_move_iterator(result.end()));
            }
        );
      }
    },
    [=](execute_phase_4) {
      self->send(dispatcher, finish::value);
    },
  };
}

#pragma once

#include <algorithm>
#include <random>
#include "../core.hpp"

using namespace caf;

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename global_temination_check,
    typename survival_selection_operator, typename elitism_operator>
struct island_model_worker_state : public base_state {
  std::size_t id;
  std::string group_name;

  island_model_worker_state() = default;

  island_model_worker_state(const shared_config& config, std::size_t id,
                            std::string group_name)
      : base_state { config },
        id { id },
        group_name { group_name } {
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename global_temination_check,
    typename survival_selection_operator, typename elitism_operator>
behavior island_model_worker(
    stateful_actor<
        island_model_worker_state<individual, fitness_value,
            fitness_evaluation_operator, initialization_operator,
            crossover_operator, mutation_operator, parent_selection_operator,
            global_temination_check, survival_selection_operator,
            elitism_operator>>* self,
    island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator, parent_selection_operator,
        global_temination_check, survival_selection_operator, elitism_operator> state,
    const actor& dispatcher) {
  self->state = std::move(state);

  auto group = self->system().groups().get_local(self->state.group_name);
  self->join(group);

  return {

  };
}

template<typename individual, typename fitness_value>
struct island_model_dispatcher_state : public base_state {
  std::size_t islands_number;
  std::string group_name;
  std::unordered_map<std::size_t, actor> islands;

  group islands_group;

  island_model_dispatcher_state() = default;

  island_model_dispatcher_state(const shared_config& config,
                                std::size_t islands_number = 1,
                                std::string group_name = "islands")
      : base_state { config },
        islands_number { islands_number },
        group_name { group_name } {
  }
};

template<typename individual, typename fitness_value,
    typename fitness_evaluation_operator, typename initialization_operator,
    typename crossover_operator, typename mutation_operator,
    typename parent_selection_operator, typename global_temination_check,
    typename survival_selection_operator, typename elitism_operator>
behavior island_model_dispatcher(
    stateful_actor<island_model_dispatcher_state<individual, fitness_value>>* self,
    island_model_dispatcher_state<individual, fitness_value> state) {
  self->state = std::move(state);
  self->state.islands_group = self->system().groups().get_local(
      self->state.group_name);

  auto spawn_worker = [self](std::size_t id) -> actor {
    return self->spawn<monitored + detached>(
        island_model_worker,
        island_model_worker_state<individual, fitness_value,
        fitness_evaluation_operator, initialization_operator,
        crossover_operator, mutation_operator,
        parent_selection_operator, global_temination_check,
        survival_selection_operator, elitism_operator> {self->state
          .config, id, self->state.group_name}, self);
  };

  for (std::size_t i = 0; i < self->state.islands_number; ++i)
    self->state.islands.emplace(i, spawn_worker(i));

  return {
    [self, spawn_worker](const down_msg& down) {
      // What should be done now? Respawn it or leave? Dependent on a policy?
      system_message(self, "Island worker with id: ", down.source.id(), " died");
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

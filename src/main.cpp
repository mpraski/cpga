#include "core.hpp"
#include "models/global_model.hpp"
#include "models/sequential_model.hpp"
#include "operators/best_individual_elitism.hpp"
#include "operators/roulette_wheel_parent_selection.hpp"
#include "operators/roulette_wheel_survival_selection.hpp"
#include "operators/sequence_individual_initialization.hpp"
#include "operators/sequence_individual_crossover.hpp"
#include "operators/average_fitness_global_termination_check.hpp"
#include "example/bitstring_mutation.cpp"
#include "example/onemax_fitness_evaluation.cpp"

int main() {
  system_properties system_props;
  system_props.population_size = 100;
  system_props.individual_size = 10;
  system_props.elitists_number = 5;
  system_props.generations_number = 100;
  system_props.initialization_seed = 0.5;
  system_props.crossover_seed = 0.5;
  system_props.mutation_seed = 0.5;
  system_props.parent_selection_seed = 0.5;
  system_props.mutation_probability = 0.015;
  system_props.supervisor_seed = 0.5;
  system_props.is_elitism_active = true;
  system_props.is_survival_selection_active = false;
  system_props.can_repeat_individual_elements = false;
  system_props.is_actor_reporter_active = true;
  system_props.actor_reporter_log = "actor_reporter.csv";

  user_properties user_props;
  user_props[constants::POSSIBLE_VALUES_KEY] = std::vector<bool> { true, false };
  user_props[constants::STABLE_REQUIRED_KEY] = std::size_t { 10 };
  user_props[constants::MINIMUM_AVERAGE_KEY] = 8;

  global_model_driver<std::vector<bool>, int, onemax_fitness_evaluation,
      sequence_individual_initialization<std::vector<bool>, int>,
      sequence_individual_crossover<std::vector<bool>, int>, bitstring_mutation,
      roulette_wheel_parent_selection<std::vector<bool>, int>,
      roulette_wheel_survival_selection<std::vector<bool>, int>,
      best_individual_elitism<std::vector<bool>, int>,
      average_fitness_global_termination_check<std::vector<bool>, int>> driver {
      system_props, user_props };

  driver.run();
}

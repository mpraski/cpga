#include "core.hpp"
#include "models/global_model.hpp"
#include "models/island_model.hpp"
#include "models/sequential_model.hpp"
#include "operators/best_individual_elitism.hpp"
#include "operators/roulette_wheel_parent_selection.hpp"
#include "operators/roulette_wheel_survival_selection.hpp"
#include "operators/sequence_individual_initialization.hpp"
#include "operators/sequence_individual_crossover.hpp"
#include "operators/average_fitness_global_termination_check.hpp"
#include "example/definitions.hpp"
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
  system_props.can_repeat_individual_elements = true;
  system_props.is_system_reporter_active = true;
  system_props.system_reporter_log = "system_reporter.csv";
  system_props.is_actor_reporter_active = true;
  system_props.actor_reporter_log = "actor_reporter.csv";
  system_props.is_individual_reporter_active = true;
  system_props.individual_reporter_log = "individual_reporter.csv";

  user_properties user_props;
  user_props[constants::POSSIBLE_VALUES_KEY] =
      std::vector<bool> { true, false };
  user_props[constants::STABLE_REQUIRED_KEY] = std::size_t { 10 };
  user_props[constants::MINIMUM_AVERAGE_KEY] = 8;

  global_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
      sequence_individual_initialization<bool, int>,
      sequence_individual_crossover<bool, int>, bitstring_mutation,
      roulette_wheel_parent_selection<sequence<bool>, int>,
      roulette_wheel_survival_selection<sequence<bool>, int>,
      best_individual_elitism<sequence<bool>, int>,
      average_fitness_global_termination_check<sequence<bool>, int>> driver {
      system_props, user_props };

  driver.run();
}

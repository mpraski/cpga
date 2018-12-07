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
#include "example/sample_migration.cpp"

int main() {
  /*
   * Core framework configuration is represented by
   * system_properties struct. See definition in core.hpp
   */
  system_properties system_props;
  system_props.islands_number = 10;
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
  system_props.is_actor_reporter_active = false;
  system_props.actor_reporter_log = "actor_reporter.csv";
  system_props.is_individual_reporter_active = true;
  system_props.individual_reporter_log = "individual_reporter.csv";

  /*
   * All other values defined by the user and somehow needed,
   * for instance in custom genetic operators (see operators/sequence_individual_initialization.hpp)
   * can be put in the user_properties map. It's a std::unordered_map<std::string, std::any>, so
   * any value can be stored (has to be cast to appropriate type using std::any_cast)
   */
  user_properties user_props;
  user_props[constants::POSSIBLE_VALUES_KEY] =
      std::vector<bool> { true, false };
  user_props[constants::STABLE_REQUIRED_KEY] = std::size_t { 10 };
  user_props[constants::MINIMUM_AVERAGE_KEY] = 8;

  /*
   * The global_model_driver class encapsulates the global model genetic algorithm.
   * When constructing a used must supply a list of types, which are as follow:
   * 1. individual type
   * 2. fitness value type
   * 3. fitness evaluation operator type
   * 4. initialization operator type
   * 5. crossover operator type
   * 6. mutatation operator type
   * 7. parent selection operator type
   * 8. survival selection operator type
   * 9. elitist operator type
   * 10. termination check operator type
   *
   * All of the 'operator' types must have a constructor
   * accepting a shared_config (shared pointer to the configuration object),
   * which allows to pass necessary data to the genetic operator functors
   */
  /*global_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
   sequence_individual_initialization<bool, int>,
   sequence_individual_crossover<bool, int>, bitstring_mutation,
   roulette_wheel_parent_selection<sequence<bool>, int>,
   roulette_wheel_survival_selection<sequence<bool>, int>,
   best_individual_elitism<sequence<bool>, int>,
   average_fitness_global_termination_check<sequence<bool>, int>> driver {
   system_props, user_props };*/

  island_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
      sequence_individual_initialization<bool, int>,
      sequence_individual_crossover<bool, int>, bitstring_mutation,
      sample_migration, roulette_wheel_parent_selection<sequence<bool>, int>,
      roulette_wheel_survival_selection<sequence<bool>, int>,
      best_individual_elitism<sequence<bool>, int>> driver { system_props,
      user_props };

  driver.run();
}

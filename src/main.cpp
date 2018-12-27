

#include "core.hpp"
#include "models/global_model.hpp"
#include "models/island_model.hpp"
#include "models/grid_model.hpp"
#include "operators/best_individual_elitism.hpp"
#include "operators/roulette_wheel_parent_selection.hpp"
#include "operators/roulette_wheel_survival_selection.hpp"
#include "operators/sequence_individual_initialization.hpp"
#include "operators/sequence_individual_crossover.hpp"
#include "operators/average_fitness_global_termination_check.hpp"
#include "example/definitions.hpp"
#include "example/bitstring_mutation.cpp"
#include "example/onemax_fitness_evaluation.hpp"
#include "operators/ring_random_migration.hpp"
#include <utilities/finite_state_machine.hpp>
#include <cluster/global_model_cluster.hpp>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

void test_global(const system_properties &sysprops, const user_properties &userprops) {
  global_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
                      sequence_individual_initialization<bool, int>,
                      sequence_individual_crossover<bool, int>, bitstring_mutation,
                      roulette_wheel_parent_selection<sequence<bool>, int>,
                      roulette_wheel_survival_selection<sequence<bool>, int>,
                      best_individual_elitism<sequence<bool>, int>,
                      average_fitness_global_termination_check<sequence<bool>, int>> driver{
      sysprops, userprops};
  driver.run();
}

void test_island(const system_properties &sysprops, const user_properties &userprops) {
  island_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
                      sequence_individual_initialization<bool, int>,
                      sequence_individual_crossover<bool, int>, bitstring_mutation,
                      roulette_wheel_parent_selection<sequence<bool>, int>,
                      roulette_wheel_survival_selection<sequence<bool>, int>,
                      best_individual_elitism<sequence<bool>, int>,
                      ring_random_migration<sequence<bool>, int>> driver{sysprops,
                                                                         userprops};
  driver.run();
}

void test_grid(const system_properties &sysprops, const user_properties &userprops) {
  grid_model_driver<sequence<bool>, int, onemax_fitness_evaluation,
                    sequence_individual_initialization<bool, int>,
                    sequence_individual_crossover<bool, int>, bitstring_mutation,
                    roulette_wheel_parent_selection<sequence<bool>, int>,
                    roulette_wheel_survival_selection<sequence<bool>, int>,
                    best_individual_elitism<sequence<bool>, int>> driver{sysprops,
                                                                         userprops};
  driver.run();
}

void test_master_node(actor_system &system,
                      const system_properties &sysprops,
                      const user_properties &userprops,
                      const cluster_properties &clusterprops) {
  cgf::cluster::master_node_driver<sequence<bool>, int, onemax_fitness_evaluation,
                                   sequence_individual_initialization<bool, int>,
                                   sequence_individual_crossover<bool, int>, bitstring_mutation,
                                   roulette_wheel_parent_selection<sequence<bool>, int>,
                                   roulette_wheel_survival_selection<sequence<bool>, int>,
                                   best_individual_elitism<sequence<bool>, int>> driver{sysprops,
                                                                                        userprops,
                                                                                        clusterprops};
  driver.run(system);
}

namespace {
void caf_main(actor_system &system, const cluster_properties &cluster_props) {
  /*
    * Core framework configuration is represented by
    * system_properties struct. See definition in core.hpp
    */
  system_properties system_props;
  system_props.islands_number = recommended_worker_number();
  system_props.population_size = 1000;
  system_props.individual_size = 10;
  system_props.elitists_number = 10;
  system_props.generations_number = 1000;
  system_props.migration_period = 10;
  system_props.migration_quota = 10;
  system_props.initialization_seed = 345312;
  system_props.crossover_seed = 654674;
  system_props.mutation_seed = 73545;
  system_props.parent_selection_seed = 764674;
  system_props.mutation_probability = 0.01;
  system_props.supervisor_seed = 876533;
  system_props.is_elitism_active = true;
  system_props.is_survival_selection_active = false;
  system_props.is_migration_active = true;
  system_props.can_repeat_individual_elements = true;
  system_props.add_island_no_to_seed = true;
  system_props.is_system_reporter_active = true;
  system_props.system_reporter_log = "system_reporter.csv";
  system_props.is_generation_reporter_active = true;
  system_props.generation_reporter_log = "generation_reporter.csv";
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
      std::vector<bool>{true, false};
  user_props[constants::STABLE_REQUIRED_KEY] = size_t{10};
  user_props[constants::MINIMUM_AVERAGE_KEY] = 8;
  user_props[constants::ADD_POPULATION_SIZE_TO_SEED] = true;

  /*
   * The global_model_driver class encapsulates the global model genetic algorithm.
   * When constructing a used must supply a list of types, which are as follow:
   * 1. individual type
   * 2. fitness value type
   * 3. fitness evaluation operator type
   * 4. initialization operator type
   * 5. crossover operator type
   * 6. mutation operator type
   * 7. parent selection operator type
   * 8. survival selection operator type
   * 9. elitist operator type
   * 10. termination check operator type
   *
   * All of the 'operator' types must have a constructor
   * accepting a shared_config (shared pointer to the configuration object),
   * which allows to pass necessary data to the genetic operator functors
   */

  cgf::cluster::run_global_model<sequence<bool>, int, onemax_fitness_evaluation,
                                 sequence_individual_initialization<bool, int>,
                                 sequence_individual_crossover<bool, int>, bitstring_mutation,
                                 roulette_wheel_parent_selection<sequence<bool>, int>,
                                 roulette_wheel_survival_selection<sequence<bool>, int>,
                                 best_individual_elitism<sequence<bool>, int>,
                                 average_fitness_global_termination_check<sequence<bool>, int>>(system,
                                                                                                system_props,
                                                                                                user_props,
                                                                                                cluster_props);
}
}

CAF_MAIN(io::middleman)
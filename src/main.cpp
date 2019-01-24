

#include "core.hpp"
#include "operators/average_fitness_global_termination_check.hpp"
#include "operators/best_individual_elitism.hpp"
#include "operators/roulette_wheel_parent_selection.hpp"
#include "operators/roulette_wheel_survival_selection.hpp"
#include "operators/sequence_individual_initialization.hpp"
#include "operators/sequence_individual_crossover.hpp"
#include "operators/ring_best_migration.hpp"
#include "example/onemax/onemax_defs.hpp"
#include "example/onemax/bitstring_mutation.hpp"
#include "example/onemax/onemax_fitness_evaluation.hpp"
#include <models/distributed/global_model_cluster.hpp>
#include <models/distributed/grid_model_cluster.hpp>
#include <models/distributed/island_model_cluster.hpp>
#include <models/single_machine/island_model_single_machine.hpp>
#include <models/single_machine/global_model_single_machine.hpp>
#include <models/single_machine/grid_model_single_machine.hpp>
#include <models/sequential_model.hpp>
#include <example/components_fault/components_fault_defs.hpp>
#include <example/components_fault/svm_fitness_evaluation.hpp>
#include <example/components_fault/svm_initialization.hpp>
#include <example/components_fault/svm_crossover.hpp>
#include <example/components_fault/svm_mutation.hpp>

using namespace caf;
using namespace std::string_literals;

void caf_main(actor_system &system, const cluster_properties &cluster_props) {
  /**
   * Core framework configuration is represented by
   * system_properties struct. See definition in core.hpp
   */
  system_properties system_props;
  // core settings
  system_props.island_model();
  system_props.total_population_size = 200;
  system_props.islands_number = recommended_worker_number();
  system_props.elitists_number = 2;
  system_props.generations_number = 200;
  system_props.migration_period = 30;
  system_props.migration_quota = 3;
  // seeds & probabilities
  system_props.initialization_seed = 345312;
  system_props.crossover_seed = 654674;
  system_props.mutation_seed = 73545;
  system_props.parent_selection_seed = 764674;
  system_props.crossover_probability = 0.5;
  system_props.mutation_probability = 0.2;
  // optional routines of a pga
  system_props.is_elitism_active = true;
  system_props.is_survival_selection_active = false;
  system_props.is_migration_active = true;
  system_props.can_repeat_individual_elements = true;
  system_props.add_island_no_to_seed = true;
  // reporters
  system_props.is_system_reporter_active = true;
  system_props.system_reporter_log = "system_reporter.csv";
  system_props.is_generation_reporter_active = true;
  system_props.generation_reporter_log = "generation_reporter.csv";
  system_props.is_individual_reporter_active = true;
  system_props.individual_reporter_log = "individual_reporter.csv";
  // need to call this to make sure population_size holds appropriate value
  system_props.compute_population_size();

  /**
   * All other values defined by the user and somehow needed,
   * for instance in custom genetic operators (see operators/sequence_individual_initialization.hpp)
   * can be put in the user_properties map. It's a std::unordered_map<std::string, std::any>, so
   * any value can be stored (has to be cast to appropriate type using std::any_cast)
   */
  user_properties user_props{
      {constants::STABLE_REQUIRED_KEY, size_t{10}},
      {constants::MINIMUM_AVERAGE_KEY, 0.9},
      {constants::CSV_FILE, "../log4j-trainset.csv"s},
      {constants::N_ROWS, 244},
      {constants::N_COLS, 9},
      {constants::N_FOLDS, 5},
      {constants::RANGE_C, std::make_pair(8.0, 32000.0)},
      {constants::RANGE_GAMMA, std::make_pair(1e-6, 1e-2)},
      {constants::MUTATION_RANGE_C, 200.0},
      {constants::MUTATION_RANGE_GAMMA, 1e-4}
  };

  /*global_cluster_runner<sequence<char>, int,
                        onemax_fitness_evaluation,
                        sequence_individual_initialization<char, int>,
                        sequence_individual_crossover<char, int>,
                        bitstring_mutation,
                        roulette_wheel_parent_selection<sequence<char>, int>,
                        roulette_wheel_survival_selection<sequence<char>, int>,
                        best_individual_elitism<sequence<char>, int>,
                        average_fitness_global_termination_check<sequence<char>, int>>::run(system,
                                                                                            system_props,
                                                                                            user_props,
                                                                                            cluster_props);*/

  /*island_cluster_runner<rbf_params, double,
                        svm_fitness_evaluation,
                        svm_initialization,
                        svm_crossover,
                        svm_mutation,
                        roulette_wheel_parent_selection<rbf_params, double>,
                        roulette_wheel_survival_selection<rbf_params, double>,
                        best_individual_elitism<rbf_params, double>,
                        ring_best_migration<rbf_params, double>>::run(system,
                                                                      system_props,
                                                                      user_props,
                                                                      cluster_props);*/

  /*grid_cluster_runner<sequence<char>, int,
                      onemax_fitness_evaluation,
                      sequence_individual_initialization<char, int>,
                      sequence_individual_crossover<char, int>,
                      bitstring_mutation,
                      roulette_wheel_parent_selection<sequence<char>, int>,
                      roulette_wheel_survival_selection<sequence<char>, int>,
                      best_individual_elitism<sequence<char>, int>>::run(system,
                                                                         system_props,
                                                                         user_props,
                                                                         cluster_props);*/

  /*grid_single_machine_runner<rbf_params, double,
                             svm_fitness_evaluation,
                             svm_initialization,
                             svm_crossover,
                             svm_mutation,
                             roulette_wheel_parent_selection<rbf_params, double>,
                             roulette_wheel_survival_selection<rbf_params, double>,
                             best_individual_elitism<rbf_params, double>>::run(system,
                                                                               system_props,
                                                                               user_props);*/
  /*global_single_machine_runner<rbf_params, double,
                               svm_fitness_evaluation,
                               svm_initialization,
                               svm_crossover,
                               svm_mutation,
                               roulette_wheel_parent_selection<rbf_params, double>,
                               roulette_wheel_survival_selection<rbf_params, double>,
                               best_individual_elitism<rbf_params, double>,
                               average_fitness_global_termination_check<rbf_params, double>>::run(system,
                                                                                                  system_props,
                                                                                                  user_props);*/

  island_single_machine_runner<rbf_params, double,
                               svm_fitness_evaluation,
                               svm_initialization,
                               svm_crossover,
                               svm_mutation,
                               roulette_wheel_parent_selection<rbf_params, double>,
                               roulette_wheel_survival_selection<rbf_params, double>,
                               best_individual_elitism<rbf_params, double>,
                               ring_best_migration<rbf_params, double>>::run(system,
                                                                             system_props,
                                                                             user_props);
}

CLUSTER_CONFIG(rbf_params, double)
CAF_MAIN(io::middleman)
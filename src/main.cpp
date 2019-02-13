#include <cpga/examples/components_fault/components_fault.hpp>
#include <cpga/models.hpp>
#include <cpga/operators.hpp>

using namespace std::string_literals;

using namespace cpga;
using namespace cpga::core;
using namespace cpga::models;
using namespace cpga::operators;
using namespace cpga::examples;

void caf_main(actor_system &system, const cluster_properties &cluster_props) {
  (void) cluster_props;
  system_properties system_props;
  // core settings
  system_props.island_model();
  system_props.total_population_size = 200;
  system_props.islands_number = recommended_worker_number();
  system_props.elitists_number = 2;
  system_props.generations_number = 50;
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

  user_properties user_props{
      {strings::STABLE_REQUIRED, size_t{10}},
      {strings::MINIMUM_AVERAGE, 0.9},
      {strings::CSV_FILE, "../../log4j-trainset.csv"s},
      {strings::N_ROWS, 244},
      {strings::N_COLS, 9},
      {strings::N_FOLDS, 5},
      {strings::RANGE_C, std::make_pair(8.0, 32000.0)},
      {strings::RANGE_GAMMA, std::make_pair(1e-6, 1e-2)},
      {strings::MUTATION_RANGE_C, 200.0},
      {strings::MUTATION_RANGE_GAMMA, 1e-4}
  };

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
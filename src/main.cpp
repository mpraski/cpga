

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

CLUSTER_CONFIG(sequence<char>, int)

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
  system_props.generations_number = 100;
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
  user_properties user_props{
      {constants::POSSIBLE_VALUES_KEY, std::vector<char>{0, 1}},
      {constants::STABLE_REQUIRED_KEY, size_t{10}},
      {constants::MINIMUM_AVERAGE_KEY, 8}
  };

  cgf::cluster::run_global_model<sequence<char>, int,
                                 onemax_fitness_evaluation,
                                 sequence_individual_initialization<char, int>,
                                 sequence_individual_crossover<char, int>,
                                 bitstring_mutation,
                                 roulette_wheel_parent_selection<sequence<char>, int>,
                                 roulette_wheel_survival_selection<sequence<char>, int>,
                                 best_individual_elitism<sequence<char>, int>,
                                 average_fitness_global_termination_check<sequence<char>, int>>(system,
                                                                                                system_props,
                                                                                                user_props,
                                                                                                cluster_props);
}
}

CAF_MAIN(io::middleman)
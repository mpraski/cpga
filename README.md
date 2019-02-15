# C++ parallel genetic algorithm framework

#### Overview

A framework for designing, running and benchmarking genetic algorithms as actor systems.

#### To-Do

- [x] Add documentation for user-facing classes
- [X] Add namespaces to separate packages
- [X] Extract headers to a dedicated directory for including as a library
- [X] Provide CMake configuration for the library
- [X] Write unit tests (leverage CAF testing DSL for actor code)
- [X] Added CMake target for generating Doxygen documentation
- [ ] Provide benchmarks

#### Example

This is still very much work in progress, so expect API changes.
Below I include an example of running the software fault detection problem on a single machine.

First, let's define the main method with few required tweaks:
```cpp
#include <models.hpp>
#include <operators.hpp>
#include <example/components_fault/components_fault.hpp>

using namespace std::string_literals;

using namespace cpga;
using namespace cpga::core;
using namespace cpga::models;
using namespace cpga::operators;
using namespace cpga::examples;

void caf_main(actor_system &system, const cluster_properties &cluster_props) {
    //...
}
CLUSTER_CONFIG(rbf_params, double)
CAF_MAIN(io::middleman)
```
The macro invocations at the bottom are necessary to properly run the algorithms in 'cluster mode' and not strictly required
for a 'single machine mode', but they can be added for the sake of later convenience.

We now define basic configuration required to run the PGA:
```cpp
void caf_main(actor_system &system, const cluster_properties &cluster_props) {
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
}
```
See the documentation of `system_properties` for parameter reference.

Having done that, the fault detection problem requires us to also pass some
auxiliary data used by custom genetic operators defined for this problem (in `example/components_fault`):
```cpp
void caf_main(actor_system &system, const cluster_properties &cluster_props) {
  //...
  user_properties user_props{
      {strings::STABLE_REQUIRED, size_t{10}},
      {strings::MINIMUM_AVERAGE, 0.9},
      {strings::CSV_FILE, "../log4j-trainset.csv"s},
      {strings::N_ROWS, 244},
      {strings::N_COLS, 9},
      {strings::N_FOLDS, 5},
      {strings::RANGE_C, std::make_pair(8.0, 32000.0)},
      {strings::RANGE_GAMMA, std::make_pair(1e-6, 1e-2)},
      {strings::MUTATION_RANGE_C, 200.0},
      {strings::MUTATION_RANGE_GAMMA, 1e-4}
  };
}
```
This is mapping from `std::string` to `std::any`, meaning you have to be careful with
the types of data you pass (notice explicit construction of `size_t{10}` - if you passed just `10` that
would default to `int` and cause a `std::bad_any_cast` to be thrown in one of the genetic operator classes.
Please see documentation for the fault detection problem genetic operator classes for additional parameter reference.
The `strings` namespace defines the string key values for convenience.

We are now ready to start the actual algorithm:
```cpp
void caf_main(actor_system &system, const cluster_properties &cluster_props) {
  //...
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
```
The `island_single_machine_runner` is an alias facilitating single-machine execution of an island model PGA (analogue for cluster model
exists too - `island_cluster_runner`). You have to pass a number of template arguments, first of which define individual and fitness_value types.
The latter define genetic operator classes to be used for this execution of the algorithm. Finally, calling `run` starts the execution of PGA. This method
returns when the execution concludes (and no living actors remain withing the system).
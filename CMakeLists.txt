cmake_minimum_required(VERSION 3.13)
project(genetic_actor)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wsign-compare")

include_directories(include)
link_directories(src)

add_executable(genetic_actor
        src/bitstring_mutation.cpp
        include/cpga/examples/onemax/onemax_defs.hpp
        include/cpga/examples/onemax/onemax_fitness_evaluation.hpp
        include/cpga/models/global_model.hpp
        include/cpga/models/grid_model.hpp
        include/cpga/models/island_model.hpp
        include/cpga/models/sequential_model.hpp
        include/cpga/operators/average_fitness_global_termination_check.hpp
        include/cpga/operators/best_individual_elitism.hpp
        include/cpga/operators/random_migration.hpp
        include/cpga/operators/ring_random_migration.hpp
        include/cpga/operators/roulette_wheel_parent_selection.hpp
        include/cpga/operators/roulette_wheel_survival_selection.hpp
        include/cpga/operators/sequence_individual_crossover.hpp
        include/cpga/operators/sequence_individual_initialization.hpp
        include/cpga/operators/star_random_migration.hpp
        include/cpga/atoms.hpp
        include/cpga/common.hpp
        include/cpga/common.cpp
        include/cpga/core.hpp
        src/main.cpp
        src/reporter.cpp
        include/cpga/utilities/reporter.hpp
        include/cpga/utilities/finite_state_machine.hpp
        include/cpga/models/distributed/global_model_cluster.hpp
        src/data.cpp include/cpga/core/data.hpp
        src/base_state.cpp include/cpga/core/base_state.hpp
        include/cpga/core/base_operator.hpp
        include/cpga/core/base_single_machine_driver.hpp include/cpga/core/defaults.hpp
        src/base_cluster_state.cpp
        include/cpga/core/base_cluster_state.hpp
        src/base_cluster_driver.cpp
        include/cpga/core/base_cluster_driver.hpp
        include/cpga/cluster/master_node.hpp
        include/cpga/cluster/reporter_node.hpp
        src/worker_node.cpp
        include/cpga/cluster/worker_node.hpp
        src/master_node.cpp
        include/cpga/cluster.hpp
        include/cpga/cluster/cluster_runner.hpp
        include/cpga/models/distributed/island_model_cluster.hpp
        src/message_bus.cpp
        include/cpga/core/message_bus.hpp
        include/cpga/models/distributed/grid_model_cluster.hpp
        include/cpga/examples/components_fault/vendor/libsvm/svm.hpp
        include/cpga/examples/components_fault/vendor/libsvm/svm.cpp
        src/svm_fitness_evaluation.cpp
        include/cpga/examples/components_fault/svm_fitness_evaluation.hpp
        include/cpga/examples/components_fault/components_fault_defs.hpp
        src/components_fault_defs.cpp
        include/cpga/examples/onemax/bitstring_mutation.hpp
        include/cpga/utilities/csv_reader.hpp
        src/onemax_fitness_evaluation.cpp
        src/svm_initialization.cpp
        include/cpga/examples/components_fault/svm_initialization.hpp
        src/svm_crossover.cpp
        include/cpga/examples/components_fault/svm_crossover.hpp
        src/svm_mutation.cpp
        include/cpga/examples/components_fault/svm_mutation.hpp
        include/cpga/operators/best_migration.hpp
        include/cpga/utilities/population_sorter.hpp
        include/cpga/operators/ring_best_migration.hpp
        include/cpga/models/single_machine/global_model_single_machine.hpp
        include/cpga/models/single_machine/grid_model_single_machine.hpp
        include/cpga/models/single_machine/island_model_single_machine.hpp
        include/cpga/core/single_machine_runner.hpp
        include/cpga/operators.hpp
        include/cpga/models.hpp
        include/cpga/examples/components_fault/components_fault.hpp
        include/cpga/examples/onemax/onemax.hpp)

target_link_libraries(genetic_actor libcaf_core.so)
target_link_libraries(genetic_actor libcaf_io.so)
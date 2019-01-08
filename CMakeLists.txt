cmake_minimum_required(VERSION 3.13)
project(genetic_actor)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(include)
include_directories(include/genetic-actor)
include_directories(src)
include_directories(src/example)
include_directories(src/models)
include_directories(src/operators)
include_directories(src/core)
include_directories(src/cluster)

add_executable(genetic_actor
        include/genetic-actor/individual.hpp
        src/example/bitstring_mutation.cpp
        src/example/definitions.hpp
        src/example/onemax_fitness_evaluation.hpp
        src/models/global_model.hpp
        src/models/grid_model.hpp
        src/models/island_model.hpp
        src/models/sequential_model.hpp
        src/operators/average_fitness_global_termination_check.hpp
        src/operators/best_individual_elitism.hpp
        src/operators/random_migration.hpp
        src/operators/ring_random_migration.hpp
        src/operators/roulette_wheel_parent_selection.hpp
        src/operators/roulette_wheel_survival_selection.hpp
        src/operators/sequence_individual_crossover.hpp
        src/operators/sequence_individual_initialization.hpp
        src/operators/star_random_migration.hpp
        src/atoms.hpp
        src/common.hpp
        src/common.cpp
        src/core.hpp
        src/main.cpp
        src/reporter.cpp
        src/reporter.hpp
        src/utilities/finite_state_machine.hpp
        src/models/distributed/global_model_cluster.hpp
        src/core/data.cpp src/core/data.hpp
        src/core/base_state.cpp src/core/base_state.hpp
        src/core/base_operator.cpp src/core/base_operator.hpp
        src/core/base_driver.hpp src/core/defaults.hpp
        src/core/base_cluster_state.cpp
        src/core/base_cluster_state.hpp
        src/core/base_cluster_driver.cpp
        src/core/base_cluster_driver.hpp
        src/cluster/master_node.hpp
        src/cluster/reporter_node.hpp
        src/cluster/worker_node.cpp
        src/cluster/worker_node.hpp
        src/cluster/master_node.cpp
        src/cluster.hpp
        src/cluster/cluster_runner.hpp
        src/models/distributed/island_model_cluster.hpp
        src/core/message_bus.cpp
        src/core/message_bus.hpp
        src/models/distributed/grid_model_cluster.hpp)

target_link_libraries(genetic_actor libcaf_core.so)
target_link_libraries(genetic_actor libcaf_io.so)
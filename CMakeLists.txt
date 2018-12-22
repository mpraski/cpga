cmake_minimum_required(VERSION 3.13)
project(genetic_actor)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(include)
include_directories(include/genetic-actor)
include_directories(src)
include_directories(src/example)
include_directories(src/models)
include_directories(src/operators)

add_executable(genetic_actor
        include/genetic-actor/individual.hpp
        src/example/bitstring_mutation.cpp
        src/example/definitions.hpp
        src/example/onemax_fitness_evaluation.cpp
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
        src/core.cpp
        src/core.hpp
        src/main.cpp
        src/reporter.cpp
        src/reporter.hpp
        src/io/node.cpp src/io/node.hpp)

target_link_libraries(genetic_actor libcaf_core.so)
target_link_libraries(genetic_actor libcaf_io.so)
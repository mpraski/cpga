find_package(Catch2 REQUIRED)

message(STATUS "Examples included")

include_directories(${genetic_actor_SOURCE_DIR}/include)
include_directories(${genetic_actor_SOURCE_DIR}/src)

file(GLOB cgpa_test_sources "*.cpp")

add_executable(cpga-test ${cgpa_test_sources} population_sorter_test.cpp common_test.cpp helpers/shared_config_builder.cpp helpers/shared_config_builder.hpp best_individual_elitism_test.cpp helpers/population_helper.cpp helpers/population_helper.hpp)
target_link_libraries(cpga-test cpga)
target_link_libraries(cpga-test Catch2::Catch2)

enable_testing()
add_test(NAME Test COMMAND cpga-test)

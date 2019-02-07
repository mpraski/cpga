find_package(Catch2 REQUIRED)

include_directories(${genetic_actor_SOURCE_DIR}/src)

add_executable (cpga-test test_main.cpp)
target_link_libraries(cpga-test cpga)
target_link_libraries(cpga-test Catch2::Catch2)

enable_testing ()
add_test (NAME Test COMMAND cpga-test)

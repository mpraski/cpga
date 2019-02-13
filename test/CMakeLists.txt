find_package(Catch2 REQUIRED)

message(STATUS "Examples included")

include_directories(${genetic_actor_SOURCE_DIR}/include)
include_directories(${genetic_actor_SOURCE_DIR}/src)

file(GLOB cgpa_test_sources *.cpp helpers/*.cpp)

add_executable(cpga-test ${cgpa_test_sources})
target_link_libraries(cpga-test cpga)
target_link_libraries(cpga-test Catch2::Catch2)

enable_testing()
add_test(NAME Test COMMAND cpga-test)

if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    setup_target_for_coverage(${PROJECT_NAME}_coverage ${TEST_TARGET} coverage)

    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif()


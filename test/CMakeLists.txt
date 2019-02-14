include(CTest)
enable_testing()

find_package(Catch2 REQUIRED)

message(STATUS "Tests included")

include_directories(${genetic_actor_SOURCE_DIR}/include)
include_directories(${genetic_actor_SOURCE_DIR}/src)

file(GLOB cgpa_test_sources "*.cpp" "helpers/*.cpp")

add_executable(cpga-test ${cgpa_test_sources})
target_link_libraries(cpga-test cpga)
target_link_libraries(cpga-test Catch2::Catch2)

add_test(NAME Test COMMAND cpga-test)

# Enable coverage is were on GCC (lcov required)
if (CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME cpga-coverage
            EXECUTABLE cpga-test
    )
    message(STATUS "Test coverage included")
endif ()
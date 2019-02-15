include_directories(${genetic_actor_SOURCE_DIR}/include)
link_directories(${genetic_actor_SOURCE_DIR}/src)

file(GLOB cgpa_sources "*.cpp")
list(REMOVE_ITEM cgpa_sources "main.cpp")

add_library(cpga ${cgpa_sources})
target_link_libraries(cpga libcaf_core.so)
target_link_libraries(cpga libcaf_io.so)

add_executable(cpga-demo main.cpp)
target_link_libraries(cpga-demo cpga)
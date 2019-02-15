cmake_minimum_required(VERSION 3.13)
project(cpga)

set(VERSION_MAJOR 0 CACHE STRING "Major version number")
set(VERSION_MINOR 0 CACHE STRING "Minor version number")
set(VERSION_PATCH 1 CACHE STRING "Patch version number")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wsign-compare")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic-errors")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(test)

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not founds")
endif()

set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${doxyfile_in} ${doxyfile} @ONLY)

add_custom_target(
        cpga-documentation
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM
)
cmake_minimum_required(VERSION 3.16)

project(Socket VERSION "1.1.0")

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# build options
option(BUILD_APPLICATION_SRC "Build application interface source files" ON)
option(BUILD_EXAMPLES_SRC "Build examples source files" ON)
option(BUILD_TESTS_SRC "Build test source files" ON)

enable_testing()

add_subdirectory(libs)
add_subdirectory(network)

if (BUILD_APPLICATION_SRC)
    add_subdirectory(application)
endif()

if (BUILD_EXAMPLES_SRC)
    add_subdirectory(examples)
endif()

if (BUILD_TESTS_SRC)
    add_subdirectory(test)
endif()

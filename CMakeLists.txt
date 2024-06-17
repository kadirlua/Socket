cmake_minimum_required(VERSION 3.16)

project(Socket)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#enable_testing()

add_subdirectory(libs)
add_subdirectory(network)
add_subdirectory(application)
add_subdirectory(examples)

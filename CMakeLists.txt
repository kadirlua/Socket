cmake_minimum_required(VERSION 3.16)

project(Socket)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_DIR ${CMAKE_SOURCE_DIR})

#enable_testing()

add_subdirectory(general)
add_subdirectory(network)
add_subdirectory(examples)
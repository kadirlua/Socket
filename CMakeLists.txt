cmake_minimum_required(VERSION 3.16)

project(Socket)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(OPENSSL_VAL 0) 

add_compile_definitions(OPENSSL_SUPPORTED=${OPENSSL_VAL})

add_subdirectory(general)
add_subdirectory(network)
add_subdirectory(examples)
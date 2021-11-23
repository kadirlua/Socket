cmake_minimum_required(VERSION 3.16)

project(Socket)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_DIR})
set(PROJECT_SOURCE_DIR ${PROJECT_DIR}/network)
set(PROJECT_EXAMPLE_DIR ${PROJECT_DIR}/examples)
set(PROJECT_GENERAL_DIR ${PROJECT_DIR}/general)
set(OPENSSL_VAL 0) 

add_compile_definitions(OPENSSL_SUPPORTED=${OPENSSL_VAL})

if (${OPENSSL_VAL})
find_package(OpenSSL REQUIRED)
endif()

set(PROJECT_SOURCES
        ${PROJECT_SOURCE_DIR}/Socket.cpp
        ${PROJECT_SOURCE_DIR}/SecureSocket.cpp
        ${PROJECT_SOURCE_DIR}/SocketObject.cpp
        ${PROJECT_SOURCE_DIR}/SecureSocketObject.cpp
        ${PROJECT_SOURCE_DIR}/SocketOption.cpp
        ${PROJECT_EXAMPLE_DIR}/Client.cpp
        ${PROJECT_EXAMPLE_DIR}/pcout.cpp
        ${PROJECT_EXAMPLE_DIR}/SecureClient.cpp
        ${PROJECT_EXAMPLE_DIR}/Server.cpp
        ${PROJECT_EXAMPLE_DIR}/SecureServer.cpp
        ${PROJECT_GENERAL_DIR}/BaseException.cpp
        ${PROJECT_GENERAL_DIR}/SocketException.cpp
        ${PROJECT_EXAMPLE_DIR}/SocketExample.cpp
        )

set(PROJECT_HEADERS
        ${PROJECT_INCLUDE_DIR}/Socket.h
        ${PROJECT_INCLUDE_DIR}/SecureSocket.h
        ${PROJECT_INCLUDE_DIR}/SocketObject.h
        ${PROJECT_INCLUDE_DIR}/SecureSocketObject.h
        ${PROJECT_INCLUDE_DIR}/SocketOption.h
        ${PROJECT_EXAMPLE_DIR}/Client.h
        ${PROJECT_EXAMPLE_DIR}/pcout.h
        ${PROJECT_EXAMPLE_DIR}/SecureClient.h
        ${PROJECT_EXAMPLE_DIR}/Server.h
        ${PROJECT_EXAMPLE_DIR}/SecureServer.h
        ${PROJECT_GENERAL_DIR}/BaseException.h
        ${PROJECT_GENERAL_DIR}/SocketException.h
        )

include_directories(${PROJECT_INCLUDE_DIR}
${OPENSSL_INCLUDE_DIR}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} -lpthread)
if (${OPENSSL_VAL})
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL)
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} Ws2_32 crypt32)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()
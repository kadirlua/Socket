set(PROJECT_NAME SocketExample)
set(PROJECT_EXAMPLES_DIR ${PROJECT_DIR}/examples)

set(PROJECT_EXAMPLES_SOURCES
    ${PROJECT_EXAMPLES_DIR}/Client.cpp
    ${PROJECT_EXAMPLES_DIR}/pcout.cpp
    ${PROJECT_EXAMPLES_DIR}/SecureClient.cpp
    ${PROJECT_EXAMPLES_DIR}/Server.cpp
    ${PROJECT_EXAMPLES_DIR}/SecureServer.cpp
    ${PROJECT_EXAMPLES_DIR}/SocketExample.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_EXAMPLES_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DIR})

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/network)

target_link_libraries(${PROJECT_NAME} PUBLIC Socket)

#add_test(SocketTest ${PROJECT_NAME})
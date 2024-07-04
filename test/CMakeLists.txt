set(PROJECT_NAME SocketTest)
set(PROJECT_TEST_DIR ${PROJECT_DIR}/test)

set(PROJECT_TEST_SOURCES
    ${PROJECT_TEST_DIR}/SocketTest.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_TEST_SOURCES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:__cplusplus")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DIR} ${PROJECT_DIR}/libs/general)

target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/network)

target_link_libraries(${PROJECT_NAME} PRIVATE Socket)

if (WIN32 AND BUILD_SHARED_LIBS)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
  COMMAND_EXPAND_LISTS
)
endif()

add_test(SocketClientServerTest ${PROJECT_NAME})
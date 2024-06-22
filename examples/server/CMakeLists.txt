set(PROJECT_EXAMPLES_SERVER_DIR ${PROJECT_DIR}/examples/server)

if (BUILD_WITH_OPENSSL)
  add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (WIN32)
  add_compile_definitions(NOMINMAX)
endif()

add_executable(ServerApp ${PROJECT_EXAMPLES_SERVER_DIR}/ServerApp.cpp)
add_executable(SSLServerApp ${PROJECT_EXAMPLES_SERVER_DIR}/SSLServerApp.cpp)

include_directories(${PROJECT_DIR})

link_directories(${CMAKE_BINARY_DIR}/network)
link_directories(${CMAKE_BINARY_DIR}/application/server)

target_link_libraries(ServerApp PRIVATE Server)
target_link_libraries(SSLServerApp PRIVATE Server)

if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ServerApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ServerApp> $<TARGET_RUNTIME_DLLS:ServerApp>
    COMMAND_EXPAND_LISTS
  )
  add_custom_command(TARGET SSLServerApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:SSLServerApp> $<TARGET_RUNTIME_DLLS:SSLServerApp>
    COMMAND_EXPAND_LISTS
  )
endif()
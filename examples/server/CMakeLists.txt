set(PROJECT_EXAMPLES_DIR ${PROJECT_DIR}/examples/server)

if (BUILD_WITH_OPENSSL)
  add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (WIN32)
  add_compile_definitions(NOMINMAX)
endif()

add_executable(ServerApp ${PROJECT_EXAMPLES_DIR}/ServerApp.cpp)

target_include_directories(ServerApp PUBLIC ${PROJECT_DIR})

target_link_directories(ServerApp PUBLIC ${CMAKE_BINARY_DIR}/network)
target_link_directories(ServerApp PUBLIC ${CMAKE_BINARY_DIR}/application/server)

target_link_libraries(ServerApp PUBLIC Socket Server)

if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ServerApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ServerApp> $<TARGET_RUNTIME_DLLS:ServerApp>
    COMMAND_EXPAND_LISTS
  )
endif()
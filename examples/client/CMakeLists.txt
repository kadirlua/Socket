set(PROJECT_EXAMPLES_DIR ${PROJECT_DIR}/examples/client)

if (BUILD_WITH_OPENSSL)
  add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (WIN32)
  add_compile_definitions(NOMINMAX)
endif()

add_executable(ClientApp ${PROJECT_EXAMPLES_DIR}/ClientApp.cpp)

target_include_directories(ClientApp PUBLIC ${PROJECT_DIR})

target_link_directories(ClientApp PUBLIC ${CMAKE_BINARY_DIR}/network)
target_link_directories(ClientApp PUBLIC ${CMAKE_BINARY_DIR}/application/client)

target_link_libraries(ClientApp PUBLIC Socket Client)

if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ClientApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ClientApp> $<TARGET_RUNTIME_DLLS:ClientApp>
    COMMAND_EXPAND_LISTS
  )
endif()
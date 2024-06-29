set(PROJECT_EXAMPLES_CLIENT_DIR ${PROJECT_DIR}/examples/client)

if (WIN32)
  add_compile_definitions(NOMINMAX)
endif()

add_executable(ClientApp ${PROJECT_EXAMPLES_CLIENT_DIR}/ClientApp.cpp)

if (BUILD_WITH_OPENSSL)
  add_executable(SSLClientApp ${PROJECT_EXAMPLES_CLIENT_DIR}/SSLClientApp.cpp)
endif()

include_directories(${PROJECT_DIR} ${PROJECT_DIR}/libs/general ${OPENSSL_INCLUDE_DIR})

link_directories(${CMAKE_BINARY_DIR}/network)
link_directories(${CMAKE_BINARY_DIR}/application/client)

target_link_libraries(ClientApp PRIVATE Client)

if (BUILD_WITH_OPENSSL)
  target_link_libraries(SSLClientApp PRIVATE Client)
endif()

if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ClientApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ClientApp> $<TARGET_RUNTIME_DLLS:ClientApp>
    COMMAND_EXPAND_LISTS
  )

  if (BUILD_WITH_OPENSSL)
    add_custom_command(TARGET SSLClientApp POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:SSLClientApp> $<TARGET_RUNTIME_DLLS:SSLClientApp>
      COMMAND_EXPAND_LISTS
    )
  endif()
endif()
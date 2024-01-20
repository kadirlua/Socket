set(PROJECT_EXAMPLES_CLIENT_DIR ${PROJECT_DIR}/examples/client)

if (BUILD_WITH_OPENSSL)
  add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (WIN32)
  add_compile_definitions(NOMINMAX)
endif()

add_executable(ClientApp ${PROJECT_EXAMPLES_CLIENT_DIR}/ClientApp.cpp)
add_executable(SSLClientApp ${PROJECT_EXAMPLES_CLIENT_DIR}/SSLClientApp.cpp)

include_directories(${PROJECT_DIR})

link_directories(${CMAKE_BINARY_DIR}/network)
link_directories(${CMAKE_BINARY_DIR}/application/client)

target_link_libraries(ClientApp PUBLIC Socket Client)
target_link_libraries(SSLClientApp PUBLIC Socket Client)

if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ClientApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ClientApp> $<TARGET_RUNTIME_DLLS:ClientApp>
    COMMAND_EXPAND_LISTS
  )
  add_custom_command(TARGET SSLClientApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:SSLClientApp> $<TARGET_RUNTIME_DLLS:SSLClientApp>
    COMMAND_EXPAND_LISTS
  )
endif()
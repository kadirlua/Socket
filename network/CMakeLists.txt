set(LIBRARY_NAME Socket)
set(PROJECT_NETWORK_DIR ${PROJECT_DIR}/network)

set(PROJECT_NETWORK_SOURCES
    ${PROJECT_NETWORK_DIR}/Socket.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocket.cpp
    ${PROJECT_NETWORK_DIR}/SocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SocketOption.cpp
)

add_library(${LIBRARY_NAME} STATIC ${PROJECT_NETWORK_SOURCES})

if (${OPENSSL_VAL})
find_package(OpenSSL REQUIRED)
endif()

if(WIN32)
  target_link_libraries(${LIBRARY_NAME} Ws2_32 crypt32 BaseException)
else()
  find_package(Threads)
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT} BaseException)
endif()

if(MSVC)
    target_compile_options(${LIBRARY_NAME} PUBLIC "/Zc:__cplusplus")
endif()

target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR})

target_link_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_BINARY_DIR}/general)
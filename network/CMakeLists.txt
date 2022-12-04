set(LIBRARY_NAME Socket)
set(PROJECT_NETWORK_DIR ${PROJECT_DIR}/network)

set(PROJECT_NETWORK_SOURCES
    ${PROJECT_NETWORK_DIR}/Socket.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocket.cpp
    ${PROJECT_NETWORK_DIR}/SocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SocketOption.cpp
)

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_MSVC_STATIC_RT TRUE)

find_package(OpenSSL QUIET)

add_library(${LIBRARY_NAME} STATIC ${PROJECT_NETWORK_SOURCES})

message("OpenSSL Found: ${OPENSSL_FOUND} with version: ${OPENSSL_VERSION}")

add_compile_definitions(OPENSSL_SUPPORTED=${OPENSSL_FOUND})

if(WIN32)
    if (OPENSSL_FOUND)
        target_link_libraries(${LIBRARY_NAME} PUBLIC OpenSSL::SSL Ws2_32 crypt32 BaseException)
    else()
        target_link_libraries(${LIBRARY_NAME} PUBLIC Ws2_32 crypt32 BaseException)
    endif()
else()
    if (OPENSSL_FOUND)
        target_link_libraries(${LIBRARY_NAME} PUBLIC OpenSSL::SSL BaseException)
    else()
        target_link_libraries(${LIBRARY_NAME} PUBLIC BaseException)
    endif()
endif()

if(MSVC)
    target_compile_options(${LIBRARY_NAME} PUBLIC "/Zc:__cplusplus")
endif()

if (OPENSSL_FOUND)
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR} ${OPENSSL_INCLUDE_DIR})
else()
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR})
endif()

target_link_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_BINARY_DIR}/general)
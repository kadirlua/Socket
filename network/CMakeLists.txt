set(LIBRARY_NAME Socket)
set(PROJECT_NETWORK_DIR ${PROJECT_DIR}/network)

set(PROJECT_NETWORK_SOURCES
    ${PROJECT_NETWORK_DIR}/Socket.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocket.cpp
    ${PROJECT_NETWORK_DIR}/SocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SocketOption.cpp
)

set(LIB_SHARED_ENABLED_BY_DEFAULT OFF)
set(LIB_STATIC_ENABLED_BY_DEFAULT ON)

option(LIB_SHARED "Build a shared version of the library" ${LIB_SHARED_ENABLED_BY_DEFAULT})
option(LIB_STATIC "Build a static version of the library" ${LIB_STATIC_ENABLED_BY_DEFAULT})

find_package(OpenSSL)

if(LIB_SHARED)
    add_library(${LIBRARY_NAME} SHARED ${PROJECT_NETWORK_SOURCES})
elseif(LIB_STATIC)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    if (MSVC)
        set(OPENSSL_MSVC_STATIC_RT TRUE)
    endif()
    add_library(${LIBRARY_NAME} STATIC ${PROJECT_NETWORK_SOURCES})
endif()

message("OpenSSL Found: ${OPENSSL_FOUND} with version: ${OPENSSL_VERSION}")
message("Library Static: ${LIB_STATIC}, Shared: ${LIB_SHARED}")

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
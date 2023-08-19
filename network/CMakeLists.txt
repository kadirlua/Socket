set(LIBRARY_NAME Socket)
set(PROJECT_NETWORK_DIR ${PROJECT_DIR}/network)

# Socket library requires C++14 or higher features
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif()

set(PROJECT_NETWORK_SOURCES
    ${PROJECT_NETWORK_DIR}/Socket.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocket.cpp
    ${PROJECT_NETWORK_DIR}/SocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SecureSocketObject.cpp
    ${PROJECT_NETWORK_DIR}/SocketOption.cpp
)

# build options
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_WITH_OPENSSL "Build with openssl support" OFF)

if (BUILD_WITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
    add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (BUILD_SHARED_LIBS)
    add_library(${LIBRARY_NAME} SHARED ${PROJECT_NETWORK_SOURCES})
    if (WIN32)
		add_compile_definitions(SOCKET_COMPILE_DLL=1)
		target_compile_definitions(${LIBRARY_NAME} PRIVATE "SOCKET_DLL_EXPORT")
	endif()
else()
    add_library(${LIBRARY_NAME} STATIC ${PROJECT_NETWORK_SOURCES})
endif()

if (OPENSSL_FOUND)
    if (WIN32)
        target_link_libraries(${LIBRARY_NAME} PUBLIC OpenSSL::SSL Ws2_32 crypt32 BaseException)
    else()
        target_link_libraries(${LIBRARY_NAME} PUBLIC OpenSSL::SSL BaseException)
    endif()
else()
    if (WIN32)
        target_link_libraries(${LIBRARY_NAME} PUBLIC Ws2_32 BaseException)
    else()
        target_link_libraries(${LIBRARY_NAME} PUBLIC BaseException)
    endif()
endif()

if (MSVC)
    target_compile_options(${LIBRARY_NAME} PUBLIC "/Zc:__cplusplus")
endif()

if (OPENSSL_FOUND)
    target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR} ${OPENSSL_INCLUDE_DIR})
else()
    target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR})
endif()

target_link_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_BINARY_DIR}/general)
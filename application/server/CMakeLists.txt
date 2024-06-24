set(LIBRARY_NAME Server)
set(PROJECT_SERVER_DIR ${PROJECT_DIR}/application/server)

set(PROJECT_SERVER_SOURCES
    ${PROJECT_SERVER_DIR}/Server.cpp
)

# Check if OpenSSL support is enabled
if (BUILD_WITH_OPENSSL)
    list(APPEND PROJECT_SERVER_SOURCES ${PROJECT_SERVER_DIR}/SSLServer.cpp)
endif()

if (BUILD_WITH_OPENSSL)
  add_compile_definitions(OPENSSL_SUPPORTED=1)
endif()

if (BUILD_SHARED_LIBS)
    add_library(${LIBRARY_NAME} SHARED ${PROJECT_SERVER_SOURCES})
    if (WIN32)
		add_compile_definitions(SOCKET_COMPILE_DLL=1)
		target_compile_definitions(${LIBRARY_NAME} PRIVATE "SOCKET_DLL_EXPORT")
	endif()
else()
    add_library(${LIBRARY_NAME} STATIC ${PROJECT_SERVER_SOURCES})
endif()

target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_DIR})

target_link_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_BINARY_DIR}/network)

target_link_libraries(${LIBRARY_NAME} PUBLIC Socket)
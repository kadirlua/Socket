set(LIBRARY_NAME Socket)
set(PROJECT_NETWORK_DIR ${PROJECT_SOURCE_DIR}/network)
set(PROJECT_LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)

# Socket library requires C++11 or higher features
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Enforce C++11 standard

set(PROJECT_NETWORK_SOURCES
    ${PROJECT_NETWORK_DIR}/SocketException.cpp
    ${PROJECT_NETWORK_DIR}/Socket.cpp
    ${PROJECT_NETWORK_DIR}/SocketDescriptor.cpp
    ${PROJECT_NETWORK_DIR}/SocketOption.cpp
)

# Check if OpenSSL support is enabled
if (BUILD_WITH_OPENSSL)
    list(APPEND PROJECT_NETWORK_SOURCES 
        ${PROJECT_NETWORK_DIR}/SSLSocket.cpp ${PROJECT_NETWORK_DIR}/SSLSocketDescriptor.cpp)
endif()

# build options
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_WITH_OPENSSL "Build with openssl support" OFF)

if (BUILD_SHARED_LIBS)
    add_library(${LIBRARY_NAME} SHARED ${PROJECT_NETWORK_SOURCES})
    if (WIN32)
		add_compile_definitions(SOCKET_COMPILE_DLL=1)
		target_compile_definitions(${LIBRARY_NAME} PRIVATE "SOCKET_DLL_EXPORT")
	endif()
else()
    add_library(${LIBRARY_NAME} STATIC ${PROJECT_NETWORK_SOURCES})
endif()

# Set the library version properties
set_target_properties(${LIBRARY_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# configure header template
configure_file(
    "${PROJECT_SOURCE_DIR}/network/version.h.in"
    "${PROJECT_BINARY_DIR}/network/version.h"
)

if (BUILD_WITH_OPENSSL)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC OPENSSL_SUPPORTED)
endif()

if (ANDROID)
    if (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
        set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/vcpkg_installed/arm-neon-android)
    elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
        set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/vcpkg_installed/arm64-android)
    elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86")
        set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/vcpkg_installed/x86-android)
    elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86_64")
        set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/vcpkg_installed/x64-android)
    endif()

    target_include_directories(${LIBRARY_NAME} PUBLIC ${OPENSSL_ROOT_DIR}/include)

    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.a)
endif()

if (BUILD_WITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
endif()

target_link_libraries(${LIBRARY_NAME} PUBLIC BaseException 
        PRIVATE $<$<TARGET_EXISTS:OpenSSL::SSL>:OpenSSL::SSL> $<$<PLATFORM_ID:Windows>:Ws2_32>)

if (MSVC)
    target_compile_options(${LIBRARY_NAME} PRIVATE "/Zc:__cplusplus")
endif()

target_include_directories(${LIBRARY_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_LIBS_DIR}/general)

include(CPack)

# Set packaging details
set(CPACK_PACKAGE_NAME "Socket")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Cross platform secure and non-secure versions of Socket classes were written in modern C++.")
set(CPACK_PACKAGE_MAINTAINER "kadirlua")

# Use file(GLOB ...) to match specific header files
file(GLOB HEADER_FILES "${PROJECT_SOURCE_DIR}/network/*.h")

# Install the matched header files
install(FILES ${HEADER_FILES} DESTINATION include)

install(FILES ${PROJECT_BINARY_DIR}/network/version.h DESTINATION include)

# Include the CHANGELOG.md and LICENSE files in the package
install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG.md DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION .)

# Include other necessary files and targets
install(TARGETS ${LIBRARY_NAME} BaseException
    EXPORT SocketTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# Export the targets for other projects to find
install(EXPORT SocketTargets
        FILE SocketTargets.cmake
        NAMESPACE Socket::
        DESTINATION lib/cmake/Socket)

# Create a config file for find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SocketConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${PROJECT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/SocketConfig.cmake"
  INSTALL_DESTINATION lib/cmake/Socket
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/SocketConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/SocketConfigVersion.cmake"
  DESTINATION lib/cmake/Socket
)

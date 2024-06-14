cmake_minimum_required(VERSION 3.22.1)

project("SocketAndroid")

# Sets LIB_SOCKET_DIR to the path of the target CMake project.
set(LIB_SOCKET_DIR ../../../../../../Socket)
# Sets LIB_SOCKET_BUILD_DIR to the path of the desired output directory.
set(LIB_SOCKET_BUILD_DIR ${LIB_SOCKET_DIR}/outputs)

# Adds the CMakeLists.txt file located in the specified directory
# as a build dependency.
add_subdirectory( # Specifies the directory of the CMakeLists.txt file.
        ${LIB_SOCKET_DIR}
        # Specifies the directory for the build outputs.
        ${LIB_SOCKET_BUILD_DIR})

include_directories(${LIB_SOCKET_DIR})

add_library(${CMAKE_PROJECT_NAME} SHARED
    SocketAndroid.cpp
)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        Socket)
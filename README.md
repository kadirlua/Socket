# Socket
Secure and non-secure versions of Socket classes. If you want to use secure connections, you have to install OpenSSL.

# Features
- Cross-platform (Windows, macOS, Linux, Android)
- C++11 and later are supported.
- Plain socket connections
- TLS/SSL connections
- TCP/UDP
- Blocking/Non-blocking mode
- Socket options

# Prerequisites
- C++11 or later supported compiler
    - msvc
    - gcc
    - clang
- Third-party libraries
    - OpenSSL (3.x.x) (Optional)
- CMake (Optional)

# Cloning the library
You can clone the library using git. This library also includes a submodule named Exception in the repository. So, for cloning the library:

```
  > git clone --recursive https://github.com/kadirlua/Socket.git
```

It's also possible to clone the library using with:

```
  > git submodule init
  > git submodule update
  > git clone https://github.com/kadirlua/Socket.git
```

# Building the library
You can build the library using vcpkg or your own environment. You can use Visual Studio, VSCode or CLion IDEs for building.

## Compile the library using cmake
```
  > mkdir build
  > cmake -B build -S .
  > cmake --build build
```
Or with make option
```
  > mkdir build
  > cd build
  > cmake ..
  > make
```

If you want to enable OpenSSL support, pass -DBUILD_WITH_OPENSSL=ON option to cmake for configuration, as shown below:
```
  > cmake -B build -S . -DBUILD_WITH_OPENSSL=ON
```

You also can build as static library (default is shared) by passing:
```
  > cmake -B build -S . -DBUILD_WITH_OPENSSL=ON -DBUILD_SHARED_LIBS=OFF
```

## Compile OpenSSL library on Windows
Before compile OpenSSL you need Strawberry Perl that you can download and install from: https://strawberryperl.com/. You also need nasm assembler which can be downloaded and installed from: https://www.nasm.us/

1. Download the lastest (v.3.x.x) source files from: https://www.openssl.org/source/.
2. Extract the zipped file to the local disk (e.g, 'C:\openssl')
3. Run the Native Tools Command Prompt for VS (x86 or x64, depending on the architecture to be compiled) as Administrator.
4. Type the following commands step by step:
```bash
cd C:/openssl
perl configure VC-WIN32 no-shared (for x86)
perl configure VC-WIN64A no-shared (for x64)
nmake
nmake install
```
5. The last two steps may take a while. So you can take a cup of coffee or tea and relax :)

## Build options
| Option                | Description                                                                  |
|-----------------------|------------------------------------------------------------------------------|
| BUILD_SHARED_LIBS     | Enables/disables shared library. Default is ON.                              |
| BUILD_WITH_OPENSSL    | Enables/disables openssl support. Default is OFF.                            |
| BUILD_EXAMPLES_SRC    | Enables/disables to build examples source codes. Default is ON.              |
| BUILD_APPLICATION_SRC | Enables/disables to build application interface source codes. Default is ON. |
| BUILD_TESTS_SRC       | Enables/disables to build test source codes. Default is ON.                  |

An example:
```
  > cmake -B build -S . -DBUILD_SHARED_LIBS=OFF -DBUILD_EXAMPLES_SRC=OFF -DBUILD_APPLICATION_SRC=ON -DBUILD_TESTS_SRC=OFF
```

## Using vcpkg
First, you have to install vcpkg in your local machine. For installing, follow these steps:
  ```
  > git clone https://github.com/microsoft/vcpkg
  > .\vcpkg\bootstrap-vcpkg.bat
  > .\vcpkg\vcpkg integrate install
  ```
After installation completed, you can search and install the required libraries as shown below:
  ```
  .\vcpkg\vcpkg install openssl --triplet=x64-windows
  ```
For Linux or macOS, change the triplet to x64-linux or x64-osx.

# Using vcpkg with CMake 
Adding the following to your workspace settings.json will make CMake Tools automatically use vcpkg for libraries:
  ```json
  {
    "cmake.configureSettings": {
      "CMAKE_TOOLCHAIN_FILE": "[vcpkg root]/scripts/buildsystems/vcpkg.cmake"
    }
  }
  ```
# Using VSCode
For using VSCode, create a new folder named '.vscode' in the project root if it does not exist. Create a new file named 'settings.json' in the '.vscode' folder as shown below:
  ```json
  {
    "json.schemaDownload.enable": true,
    "cmake.configureArgs": ["-DVCPKG_TARGET_TRIPLET=x64-windows", "-DVCPKG_ROOT=${env:USERPROFILE}/vcpkg" ,"-DBUILD_WITH_OPENSSL=ON", "-DBUILD_SHARED_LIBS=OFF"],
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "${env:USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        },
    "C_Cpp.codeAnalysis.clangTidy.enabled": true,
    "C_Cpp.codeAnalysis.runAutomatically": true
  }
  ```
You can configure the project with the arguments described above.
  ## Debugging with VSCode
  For debugging in VSCode, create a new file 'launch.json' in the .vscode folder. Specify the arguments as shown below:
  ```json
  {
    "configurations": [
        {
            "name": "C++ Test Launch",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}\\build\\examples\\Client\\ClientApp.exe",
            "args": ["www.google.com", "80", "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n"],
            "environment": [{ "name": "config", "value": "Debug" }],
            "cwd": "${workspaceFolder}"
        }
    ]
  }
  ```
  Do not forget to save the file you have created. After saving the file, follow 'Run and Debug' section in VSCode (Ctrl + Shift + D) and run the app.

# Use the library
You can use the library into your project. It's easy to integrate into your project using cmake configuration. Insert the necessary codes into your project as shown below:

CMakeLists.txt:
``` cmake

cmake_minimum_required(VERSION 3.22.1)

project(TestProject VERSION 1.0 LANGUAGES CXX)

find_package(Socket REQUIRED)    # It's required to find the library

add_executable(TestProject main.cpp)

target_link_libraries(TestProject PRIVATE Socket::Socket)    # link the library if It's found
```

main.cpp:

``` cpp
#include <iostream>
#include <Socket.h>
#include <SocketOption.h>
#include <SocketException.h>

int main()
{
    if (!sdk::network::Socket::WSAInit(sdk::network::WSA_VER_2_2)) {
		std::cout << "sdk::network::Socket::WSAInit failed\r\n";
		return -1;
	}

    try {
        sdk::network::Socket s{ 8080 };
        s.setIpAddress("127.0.0.1");
        sdk::network::SocketOption<sdk::network::Socket> opt{ s };
        opt.setBlockingMode(sdk::network::SocketOpt::ON);   // enable non-blocking mode
        s.connect();
        auto socketDesc = s.createSocketDescriptor(s.getSocketId());
        socketDesc->write("Hello from client!");
        std::string response;
        socketDesc->read(response);
        std::cout << "Response from server: " << response << "\r\n";
    } catch(const sdk::general::SocketException& err) {
        std::cout << err.getErrorMsg() << "\r\n";
    }

    sdk::network::Socket::WSADeinit();
    return 0;
}
```

# Basic example of usage (non-secure version):

```cpp
try {
  auto clientSocket = std::make_unique<sdk::network::Socket>(portNumber);
  clientSocket->setIpAddress("127.0.0.1");
  sdk::network::SocketOption<sdk::network::Socket> socketOpt{ *clientSocket };
  socketOpt.setBlockingMode(sdk::network::SocketOpt::ON);	//set non-blocking mode is active
  clientSocket->connect();  //connect to the server
  
  auto socketDesc = clientSocket->createSocketDescriptor(clientSocket->getSocketId());
  
  std::string response;
  socketDesc->write("Some important messages from client!");
  socketDesc->read(response);
  std::cout << "response from the server: " << response << "\n";
}
catch (const sdk::general::SocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

# Basic example of usage (secure version):

```cpp
try {
  static const char* certFile = "C:\\Program Files\\OpenSSL\\bin\\mycert.pem";
  static const char* keyFile = "C:\\Program Files\\OpenSSL\\bin\\privateKey.key";
  auto clientSSLSocket = std::make_unique<sdk::network::SSLSocket>(portNumber, connection_method::client);
  clientSSLSocket->setIpAddress("127.0.0.1");
  sdk::network::SocketOption<sdk::network::SSLSocket> socketOpt{ *clientSSLSocket };
  socketOpt.setBlockingMode(sdk::network::SocketOpt::ON);	//set non-blocking mode is active
  clientSSLSocket->connect();  //connect to the server
  
  clientSSLSocket->loadCertificateFile(certFile);
  clientSSLSocket->loadPrivateKeyFile(keyFile);
  
  auto socketSSLDesc = clientSSLSocket->createSocketDescriptor(clientSSLSocket->getSocketId());
  socketSSLDesc->connect();
  
  std::string response;
  socketSSLDesc->write("Some important messages from client!");
  socketSSLDesc->read(response);
  std::cout << "response from the server: " << response << "\n";
}
catch (const sdk::general::SecureSocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

# Conclusion
If you have any questions, please do not hesitate to ask me :)

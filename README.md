# Socket
Secure and non-secure version of Socket classes. If you want to use secure connections, you have to install OpenSSL.

# Features
- Cross platform (Windows, MacOS, Linux, Android)
- C++14 and later are supported.
- Plain socket connections
- TLS/SSL connections
- TCP/UDP
- Blocking/Non-blocking mode

# Prerequisites
- C++14 and later supported compiler
    - msvc
    - gcc
    - clang
- Third-party libraries
    - OpenSSL (3.x.x) (Optional)
- CMake (Optional)

# Build the library
You can build the library using vcpkg or your own environment. You can use Visual Studio, VSCode or CLion IDEs for building.

## Compile OpenSSL library on Windows
Before compile OpenSSL you need Strawberry Perl that you can download and install from: https://strawberryperl.com/. You also need nasm assembler which can be downloaded and installed from: https://www.nasm.us/

1. Download the lastest (v.3.x.x) source files from: https://www.openssl.org/source/.
2. Extract the zipped file to the local disk(C) (For example C:\openssl)
3. Run the Native Tools Command Prompt for VS (x86 or x64 which arch will be compiled) as Administrator.
4. Type the following commands step by step:
```bash
cd C:/openssl
perl configure VC-WIN32 no-shared (for x86)
perl configure VC-WIN64A no-shared (for x64)
nmake
nmake install
```
5. It will take a while the last two steps. So you can take a cup of coffee or tea and relax :)

## Using vcpkg
First, you have to install vcpkg in your local machine. For installing, follow the steps:
  ```
  > git clone https://github.com/microsoft/vcpkg
  > .\vcpkg\bootstrap-vcpkg.bat
  > .\vcpkg\vcpkg integrate install
  ```
After installation completed, you can search and install the required libraries above such as:
  ```
  .\vcpkg\vcpkg install openssl --triplet=x64-windows
  ```
For Linux or MacOS change the triplet x64-linux or x64-osx.

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
For using VSCode, we have to create a new folder named '.vscode' in the project root if it does not exist. Create a new file named 'settings.json' into '.vscode' folder such as:
  ```json
  {
    "json.schemaDownload.enable": true,
    "cmake.configureArgs": ["-DVCPKG_TARGET_TRIPLET=x64-windows", "-DVCPKG_ROOT=${env:USERPROFILE}/vcpkg" ,"-DBUILD_WITH_OPENSSL=ON", "-DBUILD_SHARED_LIBS=OFF"],
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "${env:USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        },
    "C_Cpp.codeAnalysis.clangTidy.enabled": true,
    "C_Cpp.codeAnalysis.runAutomatically": true,
  }
  ```
You can configure the project with the arguments described above.
  ## Debugging with VSCode
  For debugging in VSCode, create a new file 'launch.json' into .vscode folder. Specify the arguments such as:
  ```json
  {
    "configurations": [
        {
            "name": "C++ Test Launch",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}\\build\\examples\\Client\\ClientApp.exe",
            "args": ["127.0.0.1 8080 Hello"],
            "environment": [{ "name": "config", "value": "Debug" }],
            "cwd": "${workspaceFolder}"
        }
    ]
  }
  ```
  Do not forget to save the file you have created. After saving the file, follow 'Run and Debug' section in VSCode (Ctrl + Shift + D) and run the app.
# Basic example of usage non-secure version:

```cpp
try {
  auto clientSocket = std::make_unique<sdk::network::Socket>(portNumber);
  clientSocket->setIpAddress("127.0.0.1");
  SocketOption<Socket> socketOpt{ *clientSocket };
  socketOpt.setBlockingMode(1);	//set non-blocking mode is active
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

# Basic example of usage secure version:

```cpp
try {
  static const char* certFile = "C:\\Program Files\\OpenSSL\\bin\\mycert.pem";
  static const char* keyFile = "C:\\Program Files\\OpenSSL\\bin\\privateKey.key";
  auto clientSSLSocket = std::make_unique<sdk::network::SSLSocket>(portNumber, connection_method::client);
  clientSSLSocket->setIpAddress("127.0.0.1");
  SocketOption<SSLSocket> socketOpt{ *clientSSLSocket };
  socketOpt.setBlockingMode(1);	//set non-blocking mode is active
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
If you have any questions, please do not hesitate to ask me!

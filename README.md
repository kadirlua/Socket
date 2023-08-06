# Socket
Secure and non-secure version of Socket classes. You need OpenSSL for secure version. If you want to use these classes, do not forget to include SocketException and SecureSocketException which inherited from BaseException in the organization directory.

# Very basic example of usage non-secure version:

```c++
try {
  auto clientSocket = std::make_unique<sdk::network::Socket>(portNumber);
  clientSocket->setIpAddress("127.0.0.1");
  SocketOption<Socket> socketOpt{ *clientSocket };
  socketOpt.setBlockingMode(1);	//set non-blocking mode is active
  clientSocket->connect();  //connect to the server
  
  auto socketObj = clientSocket->createNewSocket(clientSocket->getSocketId());
  
  std::string response;
  socketObj->write("Some important messages from client!");
  socketObj->read(response);
  std::cout << "response from the server: " << response << "\n";
}
catch (const sdk::general::SocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

# Very basic example of usage secure version:

```c++
try {
  static const char* cert_file = "C:\\Program Files\\OpenSSL\\bin\\mycert.pem";
  static const char* key_file = "C:\\Program Files\\OpenSSL\\bin\\privateKey.key";
  auto clientSecureSocket = std::make_unique<sdk::network::SecureSocket>(portNumber, connection_method::client);
  clientSecureSocket->setIpAddress("127.0.0.1");
  SocketOption<SecureSocket> socketOpt{ *clientSecureSocket };
  socketOpt.setBlockingMode(1);	//set non-blocking mode is active
  clientSecureSocket->connect();  //connect to the server
  
  clientSecureSocket->loadCertificateFile(cert_file);
  clientSecureSocket->loadPrivateKeyFile(key_file);
  
  auto socketSecureObj = clientSecureSocket->createNewSocket(clientSecureSocket->getSocketId());
  socketSecureObj->connect();
  
  std::string response;
  socketSecureObj->write("Some important messages from client!");
  socketSecureObj->read(response);
  std::cout << "response from the server: " << response << "\n";
}
catch (const sdk::general::SecureSocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

# Compile OpenSSL library
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

# Conclusion
If you have any questions, please do not hesitate to ask me!

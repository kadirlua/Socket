# Socket
Secure and non-secure version of Socket classes. You need OpenSSL for secure version. If you want to use these classes, do not forget to include SocketException and SecureSocketException which inherited from BaseException in the organization directory.

# Very basic example of usage non-secure version:

```c++
try{
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
catch(const sdk::general::SocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

# Very basic example of usage secure version:

```c++
try{
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
catch(const sdk::general::SecureSocketException& ex)
{
  std::cout << "Err code: " << ex.getErrorCode() << ", Err Msg: " << ex.getErrorMsg() << "\n"; 
}
```

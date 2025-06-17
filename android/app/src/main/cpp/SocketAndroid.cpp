#include <jni.h>
#include <string>
#include <network/SSLSocket.h>
#include <network/SocketOption.h>
#include <network/SocketException.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sdk_socket_MainActivity_SendRequest(
	JNIEnv* env,
	jobject /* this */,
	jstring ipAddress,
	jint portNumber,
	jstring reqMsg)
{
	std::string response;
	// Convert the jstring to a C++ string
	const char* strIpAddress = env->GetStringUTFChars(ipAddress, nullptr);
	const char* strReqMsg = env->GetStringUTFChars(reqMsg, nullptr);
	try {
		sdk::network::SSLSocket socket{ portNumber, sdk::network::ConnMethod::client };
		socket.setIpAddress(strIpAddress);
		sdk::network::SocketOption<sdk::network::SSLSocket> socketOpt{ socket };
		socketOpt.setBlockingMode(sdk::network::SocketOpt::ON); // Set non-blocking mode is active
		socket.connect();
		auto socketDescriptor = socket.createSocketDescriptor(socket.getSocketId());
		(void)socketDescriptor->write(strReqMsg);
		(void)socketDescriptor->read(response);
	}
	catch (const sdk::general::SocketException& ex) {
		return env->NewStringUTF(ex.getErrorMsg().c_str());
	}
	return env->NewStringUTF(response.c_str());
}
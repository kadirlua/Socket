#include <jni.h>
#include <string>
#include <network/Socket.h>
#include <network/SocketOption.h>
#include <general/SocketException.h>

namespace {
    constexpr auto const PORT_NUMBER = 80;
    constexpr auto const IP_ADDRESS = "www.google.com";
    constexpr auto const STR_REQUEST = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_sdk_socket_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string response;
    try {
        sdk::network::Socket socket{PORT_NUMBER};
        socket.setIpAddress(IP_ADDRESS);
        socket.setInterruptCallback([](const sdk::network::Socket& socket) {
            (void)socket;
            return false;
        });
        sdk::network::SocketOption<sdk::network::Socket> socketOpt{ socket };
        socketOpt.setBlockingMode(sdk::network::SocketOpt::ON); // Set non-blocking mode is active
        socket.connect();
        auto socketDescriptor = socket.createSocketDescriptor(socket.getSocketId());
        (void)socketDescriptor->write(STR_REQUEST);
        (void)socketDescriptor->read(response);
    } catch(const sdk::general::SocketException& ex) {
        return env->NewStringUTF(ex.getErrorMsg().c_str());
    }
    return env->NewStringUTF(response.c_str());
}
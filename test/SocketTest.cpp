// MIT License

// Copyright (c) 2021-2025 kadirlua

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <network/Socket.h>
#include <network/SocketOption.h>
#include <network/SocketException.h>

namespace {
    const auto DEFAULT_LISTEN_PORT = 8080;
    const auto DEFAULT_CLIENT = 10;
    bool isServerReady = false;
    std::mutex lockReady;
    std::condition_variable condReady;

    void Listener()
    {
        try {
            std::unique_lock<std::mutex> lock{ lockReady };
            sdk::network::Socket server{ DEFAULT_LISTEN_PORT };
            server.bind();
            server.listen(DEFAULT_CLIENT);
            std::cout << "Server is started to listening...\r\n";
            
            isServerReady = true;
            lock.unlock();
            condReady.notify_one();
            const auto sockId = server.accept();
            auto serverDescriptor = server.createSocketDescriptor(sockId);
            std::string message;
            if (serverDescriptor->read(message) > 0) {
                std::cout << "Message from client: " << message << "\r\n";
                (void)serverDescriptor->write("OK");
            }
        } catch(const sdk::general::SocketException& err) {
            std::cout << err.getErrorMsg() << "\r\n";
        }
    }
}

int main()
{
    if (!sdk::network::Socket::WSAInit(sdk::network::WSA_VER_2_2)) {
		std::cout << "sdk::network::Socket::WSAInit failed\r\n";
		return -1;
	}

    std::string response;
    std::thread startServer{ Listener };

    try {
        std::unique_lock<std::mutex> lock{ lockReady };
        condReady.wait(lock, []() {
			return isServerReady;
		});
        sdk::network::Socket client{ DEFAULT_LISTEN_PORT };
        client.setIpAddress("127.0.0.1");
        sdk::network::SocketOption<sdk::network::Socket> opt{ client };
        opt.setBlockingMode(sdk::network::SocketOpt::ON);   // enable non-blocking mode
        client.connect();
        auto socketDesc = client.createSocketDescriptor(client.getSocketId());
        if (socketDesc->write("Hello from client!") > 0) {
            if (socketDesc->read(response) > 0) {
                std::cout << "Response from server: " << response << "\r\n";
            }
        }
    } catch(const sdk::general::SocketException& err) {
        std::cout << err.getErrorMsg() << "\r\n";
    }

    if (startServer.joinable()) {
        startServer.join();
    }

    sdk::network::Socket::WSADeinit();
    return !response.empty() ? 0 : -1;
}
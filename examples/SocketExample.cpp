// MIT License

// Copyright (c) 2021-2023 kadirlua

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

// SocketExample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "SecureServer.h"
#include "general/SocketException.h"
#include "pcout.h"
#include "Client.h"
#include "SecureClient.h"
#include "Server.h"
#include <algorithm>

#if OPENSSL_SUPPORTED
#define TEST_SECURE_SERVER 0
#endif // OPENSSL_SUPPORTED

#define TEST_IPv6 1

using namespace sdk::network;
using namespace sdk::application;
using namespace sdk::general;

#if (TEST_SECURE_SERVER == 1)
static const char* cert_file = "C:\\Program Files\\OpenSSL\\bin\\certificate.pem";
static const char* key_file = "C:\\Program Files\\OpenSSL\\bin\\key.key";
#endif

namespace {
	constexpr const auto WSA_VERSION = 0x202;
	constexpr const auto DEFAULT_PORT_NUMBER = 8086;
	constexpr const auto DEFAULT_SLEEP_TIME = 500;

	std::condition_variable server_cv;
	std::mutex server_mutex;

	void th_handler(std::string msg)
	{
		std::vector<unsigned char> response;

		try {

#if (TEST_SECURE_SERVER == 1)
#if (TEST_IPv6 == 1)
			SecureClient sclient{ "::1", DEFAULT_PORT_NUMBER, protocol_type::tcp, IpVersion::IPv6 };
#else
			SecureClient sclient{ "127.0.0.1", DEFAULT_PORT_NUMBER };
#endif

			sclient.setCertificateAtr(cert_file, key_file);
			sclient.connectServer();
			/*sclient.write(msg);
			sclient.read(response);*/

			sclient.write({ 'a', 'b', 'c' }); // initializer_list support!
			sclient.read(response);
#else
#if (TEST_IPv6 == 1)
			Client client{ "::1", DEFAULT_PORT_NUMBER, protocol_type::tcp, IpVersion::IPv6 };
#else
			Client client{ "127.0.0.1", DEFAULT_PORT_NUMBER };
#endif

			client.connectServer();

			///*client.write(msg);
			// client.read(response);*/

			client.write({ 'a', 'b', 'c' }); // initializer_list support!
			client.read(response);
#endif // TEST_SECURE_SERVER

			if (response.empty()) {
				pcout{} << "empty response"
						<< "\n";
			}
			else {
				const std::string str{ response.begin(), response.end() };
				pcout{} << str << "\n";
			}
		}
		catch (const sdk::general::SocketException& ex) {
			pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
		}
	}
}

//  basic example of inherited from Socket class
class MySocket : public Socket {
public:
	MySocket(int portNumber) :
		Socket(portNumber)
	{
	}

	~MySocket() override = default;

	void connect() override
	{
		std::cout << "Override this method"
				  << "\n";
	}
};

void serverfunc()
{
	try {
#if (TEST_SECURE_SERVER == 1)
#if (TEST_IPv6 == 1)
		const SecureServer server{ DEFAULT_PORT_NUMBER, protocol_type::tcp, IpVersion::IPv6 };
#else
		const SecureServer server{ DEFAULT_PORT_NUMBER };
#endif
#else
#if (TEST_IPv6 == 1)
		const Server server{ DEFAULT_PORT_NUMBER, protocol_type::tcp, IpVersion::IPv6 };
#else
		const Server server{ DEFAULT_PORT_NUMBER };
#endif
#endif

		server_cv.notify_one();
		while (true) {
			std::this_thread::sleep_for(std::chrono::microseconds(DEFAULT_SLEEP_TIME));
		}
	}
	catch (const SocketException& ex) {
		pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
	}
}

int main()
{
	if (Socket::WSA_startup_init(WSA_VERSION)) {
		/*  start server    */
#if (TEST_SECURE_SERVER == 1)
		SecureSocket::SSLLibraryInit();

		// print openssl library version number
		std::cout << SecureSocket::get_openssl_version() << "\n";
#endif
		std::thread server(&serverfunc);

		std::unique_lock<std::mutex> lock(server_mutex);
		server_cv.wait(lock);
		lock.unlock();

		/*  start clients   */

		std::vector<std::thread> myThreadsVec;

		for (unsigned int i = 1; i <= std::thread::hardware_concurrency(); i++) {
			std::string strMsg("hello i am thread ");
			strMsg += std::to_string(i);
			myThreadsVec.emplace_back(&th_handler, strMsg);
		}

		/*  join all threads    */

		std::for_each(myThreadsVec.begin(), myThreadsVec.end(),
			[](std::thread& th) {
				th.join();
			});

		server.join();

		Socket::WSA_Cleanup();
	}
}

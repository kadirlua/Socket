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

#pragma once
#include "network/SSLSocket.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <map>
#include <vector>
#include "Server.h"

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		class SecureServer {
		public:
			SecureServer(int port, network::protocol_type type = network::protocol_type::tcp, network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~SecureServer();

			// non copyable
			SecureServer(const SecureServer&) = delete;
			SecureServer& operator=(const SecureServer&) = delete;

		private:
			std::unique_ptr<network::SSLSocket> m_socket_ptr;

			void listener_thread_proc(int port_, network::protocol_type type, network::IpVersion ipVer);
			workerThread_t listener_thread;
			workerThread_t purging_thread;
		};

		static std::vector<std::unique_ptr<WorkerThread<network::SSLSocketDescriptor>>> thread_vec_;
		static std::mutex vec_mutex_;
		static std::condition_variable vec_cv_;
		static bool purging_flag_{};
#endif // OPENSSL_SUPPORTED
	}
}

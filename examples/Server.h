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
#include "network/Socket.h"
#include "network/SocketDescriptor.h"
#include <mutex>
#include <condition_variable>
#include <thread>

namespace sdk {
	namespace application {

		template <typename T>
		class WorkerThread : public std::thread {
		public:
			explicit WorkerThread(std::shared_ptr<T> obj_ptr) :
				std::thread(&WorkerThread::handle_thread_proc, this, obj_ptr)
			{
			}

			~WorkerThread()
			{
				if (joinable()) {
					join();
				}
			}

			bool isClosed() const noexcept
			{
				return close_flag;
			}

			void setFlag(bool flag) noexcept
			{
				close_flag = flag;
			}

			void handle_thread_proc(const std::weak_ptr<T>& socket_obj);

		private:
			bool close_flag{};
		};

		struct thread_deleter {
			inline void operator()(std::thread* ptr) const
			{
				if (ptr != nullptr) {
					if (ptr->joinable()) {
						ptr->join();
					}
					delete ptr;
				}
			}
		};

		using workerThread_t = std::unique_ptr<std::thread, thread_deleter>;
		class Server {
		public:
			Server(int port, network::ProtocolType type = network::ProtocolType::tcp, network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~Server();

			// non copyable
			Server(const Server&) = delete;
			Server& operator=(const Server&) = delete;

		private:
			std::unique_ptr<network::Socket> m_socket_ptr;

			void listener_thread_proc(int port_, network::ProtocolType type, network::IpVersion ipVer);
			workerThread_t listener_thread;
			workerThread_t purging_thread;
		};

		static std::vector<std::unique_ptr<WorkerThread<network::SocketDescriptor>>> thread_vec;
		static std::mutex vec_mutex;
		static std::condition_variable vec_cv;
		static bool purging_flag{};
	};
}

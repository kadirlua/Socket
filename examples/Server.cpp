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

#include "Server.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"
#include "pcout.h"

#include <algorithm>

namespace sdk {
	namespace application {

		using namespace network;

		namespace {

			constexpr const auto MAX_CLIENTS = 10;

			void thread_purging()
			{
				while (!purging_flag) {
					std::unique_lock<std::mutex> lock_(vec_mutex);
					vec_cv.wait(lock_, []() { return !thread_vec.empty() || purging_flag; });
					thread_vec.erase(std::remove_if(thread_vec.begin(), thread_vec.end(),
										 [](const auto& socketObj) {
											 return socketObj->isClosed();
										 }),
						thread_vec.end());
				}
			}
		}

		Server::Server(int port, ProtocolType type /*= ProtocolType::tcp*/, IpVersion ipVer /*= IpVersion::IPv4*/) :
			listener_thread{ new std::thread{ &Server::listener_thread_proc, this, port, type, ipVer } },
			purging_thread{ new std::thread{ &thread_purging } }
		{
		}

		Server::~Server()
		{
			vec_mutex.lock();
			purging_flag = true;
			vec_mutex.unlock();
			vec_cv.notify_all();
		}

		void Server::listener_thread_proc(int port_, network::ProtocolType type, network::IpVersion ipVer)
		{
			try {
				m_socket_ptr = std::make_unique<Socket>(port_, type, ipVer);
				//	set socket options
				const SocketOption<Socket> socketOpt{ *m_socket_ptr };
				socketOpt.setBlockingMode(1);
				// initialize server
				m_socket_ptr->bind();
				m_socket_ptr->listen(MAX_CLIENTS);

				while (true) {
					try {
						const SOCKET new_socket_id = m_socket_ptr->accept();
						auto socket_obj = m_socket_ptr->createNewSocket(new_socket_id);
						std::unique_lock<std::mutex> lock_(vec_mutex);
						thread_vec.emplace_back(new WorkerThread<SocketDescriptor>(socket_obj));
						lock_.unlock();
						vec_cv.notify_one();
					}
					catch (const general::SocketException& ex) {
						(void)ex;
					}
				}
			}
			catch (const general::SocketException& ex) {
				pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
			}
		}

		template <>
		void WorkerThread<SocketDescriptor>::handle_thread_proc(const std::weak_ptr<SocketDescriptor>& socket_obj)
		{
			auto sharedSocketPtr = socket_obj.lock();
			if (sharedSocketPtr) {
				std::string request_message;
				const std::string response = "I hear You";

				while (true) {
					try {
						sharedSocketPtr->read(request_message);
						pcout{} << request_message << "\n";
						sharedSocketPtr->write(response);
					}
					catch (const general::SocketException& ex) {
						pcout{} << ex.getErrorCode() << " read or write : "
								<< ": " << ex.getErrorMsg() << "\n";
						break;
					}
				}

				vec_mutex.lock();
				close_flag = true;
				vec_mutex.unlock();
				vec_cv.notify_one();
			}
		}
	}
}

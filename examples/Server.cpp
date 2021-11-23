#include "Server.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"
#include "pcout.h"


namespace sdk {
	namespace application {
		
		using namespace network;

		static void thread_purging()
		{
			while (!purging_flag)
			{
				std::unique_lock<std::mutex> lock_(vec_mutex);
				vec_cv.wait(lock_, []() { return !thread_vec.empty() || purging_flag; });
				for (auto iter = thread_vec.rbegin(); iter != thread_vec.rend(); iter++)
				{
					if ((*iter)->isClosed())
					{
						thread_vec.erase(iter.base() - 1);
						break;
					}
				}
			}
		}
		
		Server::Server(int port, protocol_type type /*= protocol_type::tcp*/, IpVersion ipVer /*= IpVersion::IPv4*/):
			listener_thread{ new std::thread{ &Server::listener_thread_proc, this, port, type, ipVer} },
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

		void Server::listener_thread_proc(int port_, network::protocol_type type , network::IpVersion ipVer )
		{
			try{
				m_socket_ptr = std::make_unique<Socket>(port_, type, ipVer);
				//	set socket options
				SocketOption<Socket> socketOpt{ *m_socket_ptr };
				socketOpt.setBlockingMode(1);
				// initialize server
				m_socket_ptr->bind();
				m_socket_ptr->listen(10);

				while (true) 
				{
					try {
						SOCKET new_socket_id = m_socket_ptr->accept();
						auto socket_obj = m_socket_ptr->createnewSocket(new_socket_id);
						std::unique_lock<std::mutex> lock_(vec_mutex);
						thread_vec.emplace_back(new WorkerThread<SocketObj>(socket_obj));
						lock_.unlock();
						vec_cv.notify_one();
					}
					catch (const general::SocketException& ex)
					{
						(void)ex;
					}
				}
			}
			catch (const general::SocketException& ex)
			{
				pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
			}
		}

		template<>
		void WorkerThread<SocketObj>::handle_thread_proc(std::weak_ptr<SocketObj> socket_obj)
		{
			auto sharedSocketPtr = socket_obj.lock();
			if (sharedSocketPtr)
			{
				std::string request_message;
				std::string response = "I hear You";

				while (true)
				{
					try {
						sharedSocketPtr->read(request_message);
						pcout{} << request_message << "\n";
						sharedSocketPtr->write(response);
					}
					catch (const general::SocketException& ex)
					{
						pcout{} << ex.getErrorCode() << " read or write : " << ": " << ex.getErrorMsg() << "\n";
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
#pragma once
#include "network/Socket.h"
#include "network/SocketObject.h"
#include <mutex>
#include <condition_variable>
#include <thread>


namespace sdk {
	namespace application {

		template<typename T>
		class WorkerThread : public std::thread
		{
		public:
			explicit WorkerThread(std::shared_ptr<T> obj_ptr) :
				std::thread(&WorkerThread::handle_thread_proc, this, obj_ptr)
			{
			}

			~WorkerThread() {
				if (joinable())
					join();
			}

			bool isClosed() const noexcept {
				return close_flag;
			}

			void setFlag(bool flag) noexcept {
				close_flag = flag;
			}

			void handle_thread_proc(std::weak_ptr<T> socket_obj);

		private:
			bool close_flag{};
		};

		struct thread_deleter
		{
			inline void operator()(std::thread* ptr) const
			{
				if (ptr)
				{
					if (ptr->joinable())
						ptr->join();
					delete ptr;
				}
			}
		};

		using workerThread_t = std::unique_ptr<std::thread, thread_deleter>;
		class Server
		{
		public:
			Server(int port, network::protocol_type type = network::protocol_type::tcp, network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~Server();

			//non copyable
			Server(const Server&) = delete;
			Server& operator=(const Server&) = delete;

		private:
			std::unique_ptr<network::Socket> m_socket_ptr;

			void listener_thread_proc(int port_, network::protocol_type type, network::IpVersion ipVer);
			workerThread_t listener_thread;
			workerThread_t purging_thread;
		};

		static std::vector<std::unique_ptr<WorkerThread<network::SocketObj>>> thread_vec;
		static std::mutex vec_mutex;
		static std::condition_variable vec_cv;
		static bool purging_flag{};
	};
};

#pragma once
#include "network/SecureSocket.h"
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

		class SecureServer
		{
		public:
			SecureServer(int port, network::protocol_type type = network::protocol_type::tcp, network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~SecureServer();

			//non copyable
			SecureServer(const SecureServer&) = delete;
			SecureServer& operator=(const SecureServer&) = delete;
		private:
			std::unique_ptr<network::SecureSocket> m_socket_ptr;

			void listener_thread_proc(int port_, network::protocol_type type, network::IpVersion ipVer);
			workerThread_t listener_thread;
			workerThread_t purging_thread;
		};

		static std::vector<std::unique_ptr<WorkerThread<network::SecureSocketObj>>> thread_vec_;
		static std::mutex vec_mutex_;
		static std::condition_variable vec_cv_;
		static bool purging_flag_{};
#endif // OPENSSL_SUPPORTED
	}
}

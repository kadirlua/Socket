#pragma once
#include "network/Socket.h"

namespace sdk {
	namespace application {
		class Client
		{
		public:

			Client(std::string ip, int port, network::protocol_type type = network::protocol_type::tcp, network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~Client() = default;

			//non copyable
			Client(const Client&) = delete;
			Client& operator=(const Client&) = delete;

			void connectServer();
			int write(std::initializer_list<char> msg) const;
			int write(const char* msg, int msg_size) const;
			int write(std::string& msg) const;
			int write(std::vector<unsigned char>& message) const;
			size_t read(std::vector<unsigned char>& response_msg, int max_size = 0) const;
			size_t read(std::string& message, int max_size = 0) const;
			bool isInterrupted() const noexcept {
				return m_bInterrupt;
			}

		private:
			bool m_bInterrupt{};
			std::unique_ptr<network::Socket> m_socket;
			std::shared_ptr<network::SocketObject> m_socket_obj;
		};
	};
};

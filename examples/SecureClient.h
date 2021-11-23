#pragma once
#include "network/SecureSocket.h"
#include <string>

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		class SecureClient
		{
		public:
			SecureClient(std::string ip, int port, 
				network::protocol_type type = network::protocol_type::tcp, 
				network::IpVersion ipVer = network::IpVersion::IPv4);
			virtual ~SecureClient() = default;

			//non copyable
			SecureClient(const SecureClient&) = delete;
			SecureClient& operator=(const SecureClient&) = delete;

			void setCertificateAtr(const char* cert_file, const char* key_file) const;
			void connectServer();
			int write(std::initializer_list<char> msg) const;
			int write(const char* msg, int msg_size) const;
			int write(const std::string& msg) const;
			int write(const std::vector<unsigned char>& msg) const;
			size_t read(std::vector<unsigned char>& response_msg, int max_size = 0) const;
			size_t read(std::string& message, int max_size = 0) const;
			bool isInterrupted() const noexcept {
				return m_bInterrupt;
			}

		private:
			bool m_bInterrupt{};
			std::unique_ptr<network::SecureSocket> m_socket_ptr;
			std::shared_ptr<network::SecureSocketObj> m_secure_obj;
		};
#endif // OPENSSL_SUPPORTED
	}
}

#include "SecureClient.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

		using namespace network;

#if OPENSSL_SUPPORTED

		static inline bool callbackInterrupt(void* userdata_ptr)
		{
			SecureClient* client = reinterpret_cast<SecureClient*>(userdata_ptr);
			return client->isInterrupted();
		}

		SecureClient::SecureClient(std::string ip, int port, network::protocol_type type /*= network::protocol_type::tcp*/, network::IpVersion ipVer /*= network::IpVersion::IPv4*/) :
			m_socket_ptr{std::make_unique<SecureSocket>(port, connection_method::client, type, ipVer)}

		{
			m_socket_ptr->setIpAddress(ip);
			SocketOption<SecureSocket> socketOpt{ *m_socket_ptr };
			socketOpt.setBlockingMode(1);	//non-blocking mode	
			m_socket_ptr->setInterruptCallback(&callbackInterrupt, this);
			m_socket_ptr->connect();
		}

		void SecureClient::setCertificateAtr(const char* cert_file, const char* key_file) const
		{
			m_socket_ptr->loadCertificateFile(cert_file);
			m_socket_ptr->loadPrivateKeyFile(key_file);
		}

		void SecureClient::connectServer()
		{

			m_secure_obj = m_socket_ptr->createnewSocket(m_socket_ptr->getSocketId());
			m_secure_obj->connect();
		}

		int SecureClient::write(std::initializer_list<char> msg) const
		{
			return m_secure_obj->write(msg);
		}

		int SecureClient::write(const char* msg, int msg_size) const
		{
			return m_secure_obj->write(msg, msg_size);
		}

		int SecureClient::write(const std::string& msg) const
		{
			return m_secure_obj->write(msg);
		}

		int SecureClient::write(const std::vector<unsigned char>& msg) const
		{
			return m_secure_obj->write(msg);
		}

		size_t SecureClient::read(std::vector<unsigned char>& response_msg, int max_size /*= 0*/) const
		{
			return m_secure_obj->read(response_msg, max_size);
		}

		size_t SecureClient::read(std::string& message, int max_size /*= 0*/) const
		{
			return m_secure_obj->read(message);
		}
#endif // OPENSSL_SUPPORTED
	}
}
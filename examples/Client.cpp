#include "Client.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

		using namespace network;
		
		static inline bool callbackInterrupt(void* userdata_ptr)
		{
			Client* client = reinterpret_cast<Client*>(userdata_ptr);
			return client->isInterrupted();
		}

		Client::Client(std::string ip, int port, network::protocol_type type /*= protocol_type::tcp*/, network::IpVersion ipVer /*= IpVersion::IPv4*/) :
			m_socket{std::make_unique<Socket>(port, type, ipVer)}
		{
			m_socket->setIpAddress(ip);
			m_socket->setInterruptCallback(&callbackInterrupt, this);
		}

		void Client::connectServer()
		{
			m_socket->connect();
			SocketOption<Socket> socketOpt{ *m_socket };
			socketOpt.setBlockingMode(1);
			m_socket_obj = m_socket->createnewSocket(m_socket->getSocketId());
		}

		int Client::write(std::initializer_list<char> msg) const
		{
			return m_socket_obj->write(msg);
		}

		int Client::write(const char* msg, int msg_size) const
		{
			return m_socket_obj->write(msg, msg_size);
		}

		int Client::write(std::string& msg) const
		{
			return m_socket_obj->write(msg);
		}

		int Client::write(std::vector<unsigned char>& msg) const
		{
			return m_socket_obj->write(msg);
		}

		size_t Client::read(std::vector<unsigned char>& response_msg, int max_size /*= 0*/) const
		{
			return m_socket_obj->read(response_msg, max_size);
		}

		size_t Client::read(std::string& message, int max_size /*= 0*/) const
		{
			return m_socket_obj->read(message, max_size);
		}
	}

}
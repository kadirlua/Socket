#include "Socket.h"
#include "general/SocketException.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace sdk {
	namespace network {

		bool Socket::m_wsa_init = false;

		bool Socket::WSA_startup_init(unsigned short versionReq) noexcept
		{
			if (!m_wsa_init) {
#ifdef _WIN32
				WSADATA wsaData;
				if (WSAStartup(versionReq, &wsaData))
				{
					return false;
				}
#endif
				m_wsa_init = true;
			}
			return true;
		}

		void Socket::WSA_Cleanup() noexcept
		{
#ifdef _WIN32
			if (m_wsa_init) {
				WSACleanup();
				m_wsa_init = false;
			}
#endif
		}

		Socket::Socket(int port_, protocol_type type /*= protocol_type::tcp*/, IpVersion ipVer /*= IpVersion::IPv4*/) :
			m_port_number{ port_ },
			m_protocol_type{ type },
			m_ipVersion{ ipVer }
		{
			
			if ((m_socket_id = socket(static_cast<int>(m_ipVersion), static_cast<int>(type), 0)) == INVALID_SOCKET)
			{
				throw general::SocketException(WSAGetLastError());
			}
			
			if (m_ipVersion == IpVersion::IPv4)
			{
				m_st_address_t.sin_family = AF_INET;
				m_st_address_t.sin_addr.s_addr = htonl(INADDR_ANY);
				m_st_address_t.sin_port = htons(m_port_number);
			}			
			
			if (m_ipVersion == IpVersion::IPv6)
			{
				m_st_address6_t.sin6_family = AF_INET6;
				m_st_address6_t.sin6_port = htons(m_port_number);
			}
		}

		Socket::~Socket()
		{
			shutdown(m_socket_id, SD_SEND);
			while (closesocket(m_socket_id) == SOCKET_ERROR)
			{
				auto err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
					break;
			}
		}

		std::string Socket::getIpAddress() const 
		{
			struct addrinfo hints, * res, * p;
			char ipstr[INET6_ADDRSTRLEN]{};

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
			hints.ai_socktype = SOCK_STREAM;

			if (getaddrinfo(m_ip_address.c_str(), nullptr, &hints, &res) != 0) {
				throw general::SocketException(WSAGetLastError());
			}

			for (p = res; p != nullptr; p = p->ai_next) {
				void* addr;
				
				// get the pointer to the address itself,
				// different fields in IPv4 and IPv6:
				if (p->ai_family == AF_INET) { // IPv4
					struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
					addr = &(ipv4->sin_addr);
				}
				else { // IPv6
					struct sockaddr_in6* ipv6 = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
					addr = &(ipv6->sin6_addr);
				}

				// convert the IP to a string
				if (!inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr)))
				{
					throw general::SocketException(WSAGetLastError());
				}
			}

			freeaddrinfo(res); // free the linked list
			return ipstr;
		}

		void Socket::connect()
		{
			timeval timeout{ 0, 1000 };

			int err{};
			auto strIpAddr = getIpAddress();
			
			if (strIpAddr.empty())
				throw general::SocketException("IP address is not valid.");

			if(m_ipVersion == IpVersion::IPv4)
				err = inet_pton(static_cast<int>(m_ipVersion), strIpAddr.c_str(), &m_st_address_t.sin_addr);
			if(m_ipVersion == IpVersion::IPv6)
				err = inet_pton(static_cast<int>(m_ipVersion),  strIpAddr.c_str(), &m_st_address6_t.sin6_addr);

			if (err == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
			else if (err == 0)
			{
				throw general::SocketException("IP address is not valid IPv4 dotted-decimal string or a valid IPv6 address string.");
			}			
			
			int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t));

			const sockaddr* st_address = ( m_ipVersion == IpVersion::IPv4 ?
				reinterpret_cast<const sockaddr*>(&m_st_address_t) : reinterpret_cast<const sockaddr*>(&m_st_address6_t));

			while (::connect(m_socket_id, st_address, addressSize) == SOCKET_ERROR)
			{
				//	check if any interrupt happened by user
				if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr))
					throw general::SocketException(INTERRUPT_MSG);

				const int lastError = WSAGetLastError();

				switch (lastError) {
				case WSAEWOULDBLOCK: {
					fd_set writefds{}, exceptfds{};

					do {
						if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr))
							throw general::SocketException(INTERRUPT_MSG);

						FD_ZERO(&writefds);
						FD_SET(m_socket_id, &writefds);
						FD_ZERO(&exceptfds);
						FD_SET(m_socket_id, &exceptfds);
						err = select((int)m_socket_id + 1, nullptr, &writefds, &exceptfds, &timeout);
						if (err < 0)
							throw general::SocketException(WSAGetLastError());
						if (FD_ISSET(m_socket_id, &exceptfds))
							throw general::SocketException("Cannot connect to the server");
					} while (!FD_ISSET(m_socket_id, &writefds));
					
				}
					break;
				case WSAEALREADY:
				case WSAEISCONN:
					return;
				default:
					throw general::SocketException(lastError);
				}
			}
		}

		void Socket::bind()
		{
			int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t));
			const sockaddr* st_address = (m_ipVersion == IpVersion::IPv4 ?
				reinterpret_cast<const sockaddr*>(&m_st_address_t) : reinterpret_cast<const sockaddr*>(&m_st_address6_t));
			if (::bind(m_socket_id, st_address, addressSize) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		void Socket::listen(int listen_count) const
		{
			if (m_protocol_type != protocol_type::udp)
			{
				if (::listen(m_socket_id, listen_count) == SOCKET_ERROR)
				{
					throw general::SocketException(WSAGetLastError());
				}
			}
		}

		SOCKET Socket::accept()
		{
			if (m_protocol_type != protocol_type::udp)
			{
				timeval timeout{ 0, 1000 };
				
				socklen_t addrlen = m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t);
				SOCKET new_sock_id{};
				
				 sockaddr* st_address = (m_ipVersion == IpVersion::IPv4 ?
					reinterpret_cast< sockaddr*>(&m_st_address_t) : reinterpret_cast< sockaddr*>(&m_st_address6_t));

				while ((new_sock_id = ::accept(m_socket_id, st_address,&addrlen)) == INVALID_SOCKET)
				{
					//	check if any interrupt happened by user
					if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr))
						throw general::SocketException(INTERRUPT_MSG);

					switch (auto lasterror = WSAGetLastError())
					{
					case WSAEWOULDBLOCK:
					{
						fd_set readfds{}, exceptfds{};

						do {
							if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr))
								throw general::SocketException(INTERRUPT_MSG);

							FD_ZERO(&readfds);
							FD_SET(m_socket_id, &readfds);
							FD_ZERO(&exceptfds);
							FD_SET(m_socket_id, &exceptfds);
							auto err = select((int)m_socket_id + 1, &readfds, nullptr, &exceptfds, &timeout);
							if (err < 0)
								throw general::SocketException(WSAGetLastError());
							if (FD_ISSET(m_socket_id, &exceptfds))
								throw general::SocketException("Cannot connect to the server");
						} while (!FD_ISSET(m_socket_id, &readfds));
					}
					break;
					default:
						throw general::SocketException(lasterror);
					}
				}

				return new_sock_id;
			}

			return 0;
		}

		std::shared_ptr<SocketObj> Socket::createnewSocket(SOCKET socket_id) const
		{
			return std::make_shared<SocketObj>(socket_id, *this);
		}

		void Socket::setInterruptCallback(const socket_interrupt_callback_t& callback, void * userdata) noexcept
		{
			m_callback_interrupt = callback;
			m_userdata_ptr = userdata;
		}
	}
}
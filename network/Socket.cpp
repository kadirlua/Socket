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

#include "Socket.h"
#include "general/SocketException.h"
#include <cstring>
#include <array>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace sdk {

	namespace {
		constexpr auto const DEFAULT_TIMEOUT = 1000L;
	}

	namespace network {

		bool Socket::m_wsa_init = false;

		bool Socket::WSA_startup_init(unsigned short versionReq) noexcept
		{
			// There's no harm in calling WSAStartup() multiple times but no benefit
			// either, we may as well skip it after the first.
			if (!m_wsa_init) {
#ifdef _WIN32
				WSADATA wsaData;
				if (WSAStartup(versionReq, &wsaData) != 0) {
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
			m_socket_id = socket(static_cast<int>(m_ipVersion), static_cast<int>(type), 0);
			if (m_socket_id == INVALID_SOCKET) {
				throw general::SocketException(WSAGetLastError());
			}

			if (m_ipVersion == IpVersion::IPv4) {
				m_st_address_t.sin_family = AF_INET;
				m_st_address_t.sin_addr.s_addr = htonl(INADDR_ANY);
				m_st_address_t.sin_port = htons(m_port_number);
			}
			else if (m_ipVersion == IpVersion::IPv6) {
				m_st_address6_t.sin6_family = AF_INET6;
				m_st_address6_t.sin6_port = htons(m_port_number);
			}
		}

		Socket::~Socket()
		{
			shutdown(m_socket_id, SD_SEND);
			while (closesocket(m_socket_id) == SOCKET_ERROR) {
				auto err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK) {
					break;
				}
			}
		}

		std::string Socket::getIpAddress() const
		{
			struct addrinfo hints{};
			struct addrinfo *res = nullptr;
			struct addrinfo *ptr = nullptr;

			std::array<char, INET6_ADDRSTRLEN> ipStr{};

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
			hints.ai_socktype = SOCK_STREAM;

			if (getaddrinfo(m_ip_address.c_str(), nullptr, &hints, &res) != 0) {
				throw general::SocketException(WSAGetLastError());
			}

			const std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> pResPtr{ res, freeaddrinfo }; 

			for (ptr = pResPtr.get(); ptr != nullptr; ptr = ptr->ai_next) {
				void* pAddr = nullptr;

				// get the pointer to the address itself,
				// different fields in IPv4 and IPv6:
				if (ptr->ai_family == AF_INET) { // IPv4
					auto* ipv4 = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
					pAddr = &(ipv4->sin_addr);
				}
				else { // IPv6
					auto* ipv6 = reinterpret_cast<struct sockaddr_in6*>(ptr->ai_addr);
					pAddr = &(ipv6->sin6_addr);
				}

				// convert the IP to a string
				if (inet_ntop(ptr->ai_family, pAddr, ipStr.data(), sizeof(ipStr)) == nullptr) {
					throw general::SocketException(WSAGetLastError());
				}
			}
			return std::string{ std::begin(ipStr), std::end(ipStr) };
		}

		void Socket::connect()
		{
			timeval timeout{ 0, DEFAULT_TIMEOUT };

			int err{};
			auto strIpAddr = getIpAddress();

			if (strIpAddr.empty()) {
				throw general::SocketException("IP address is not valid.");
			}

			if (m_ipVersion == IpVersion::IPv4) {
				err = inet_pton(static_cast<int>(m_ipVersion),
					strIpAddr.c_str(), &m_st_address_t.sin_addr);
			}
			else if (m_ipVersion == IpVersion::IPv6) {
				err = inet_pton(static_cast<int>(m_ipVersion),
					strIpAddr.c_str(), &m_st_address6_t.sin6_addr);
			}

			if (err == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}

			if (err == 0) {
				throw general::SocketException("IP address is not valid IPv4 dotted-decimal string or a valid IPv6 address string.");
			}

			const int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t));

			const sockaddr* st_address = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<const sockaddr*>(&m_st_address_t) : reinterpret_cast<const sockaddr*>(&m_st_address6_t));

			while (::connect(m_socket_id, st_address, addressSize) == SOCKET_ERROR) {
				//	check if any interrupt happened by user
				if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				const int lastError = WSAGetLastError();

				switch (lastError) {
				case WSAEWOULDBLOCK: {
					fd_set writeFds{};
					fd_set exceptFds{};

					do {
						if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr)) {
							throw general::SocketException(INTERRUPT_MSG);
						}

						FD_ZERO(&writeFds);
						FD_SET(m_socket_id, &writeFds);
						FD_ZERO(&exceptFds);
						FD_SET(m_socket_id, &exceptFds);
						err = select((int)m_socket_id + 1, nullptr, &writeFds, &exceptFds, &timeout);
						if (err < 0) {
							throw general::SocketException(WSAGetLastError());
						}
						if (FD_ISSET(m_socket_id, &exceptFds)) {
							throw general::SocketException("Cannot connect to the server");
						}
					} while (!FD_ISSET(m_socket_id, &writeFds));

				} break;
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
			const int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t));
			const sockaddr* st_address = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<const sockaddr*>(&m_st_address_t) : reinterpret_cast<const sockaddr*>(&m_st_address6_t));
			if (::bind(m_socket_id, st_address, addressSize) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		void Socket::listen(int listen_count) const
		{
			if (m_protocol_type != protocol_type::udp) {
				if (::listen(m_socket_id, listen_count) == SOCKET_ERROR) {
					throw general::SocketException(WSAGetLastError());
				}
			}
		}

		SOCKET Socket::accept()
		{
			if (m_protocol_type != protocol_type::udp) {
				timeval timeout{ 0, DEFAULT_TIMEOUT };

				socklen_t addrLen = m_ipVersion == IpVersion::IPv4 ? sizeof(m_st_address_t) : sizeof(m_st_address6_t);
				SOCKET new_sock_id{};

				sockaddr* st_address = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<sockaddr*>(&m_st_address_t) : reinterpret_cast<sockaddr*>(&m_st_address6_t));

				while ((new_sock_id = ::accept(m_socket_id, st_address, &addrLen)) == INVALID_SOCKET) {
					//	check if any interrupt happened by user
					if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (auto lasterror = WSAGetLastError()) {
					case WSAEWOULDBLOCK: {
						fd_set readFds{};
						fd_set exceptFds{};

						do {
							if (m_callback_interrupt && m_callback_interrupt(m_userdata_ptr)) {
								throw general::SocketException(INTERRUPT_MSG);
							}

							FD_ZERO(&readFds);
							FD_SET(m_socket_id, &readFds);
							FD_ZERO(&exceptFds);
							FD_SET(m_socket_id, &exceptFds);
							auto err = select((int)m_socket_id + 1, &readFds, nullptr, &exceptFds, &timeout);
							if (err < 0) {
								throw general::SocketException(WSAGetLastError());
							}
							if (FD_ISSET(m_socket_id, &exceptFds)) {
								throw general::SocketException("Cannot connect to the server");
							}
						} while (!FD_ISSET(m_socket_id, &readFds));
					} break;
					default:
						throw general::SocketException(lasterror);
					}
				}

				return new_sock_id;
			}

			return 0;
		}

		std::shared_ptr<SocketObject> Socket::createNewSocket(SOCKET socket_id) const
		{
			return std::make_shared<SocketObject>(socket_id, *this);
		}

		void Socket::setInterruptCallback(const socket_interrupt_callback_t& callback, void* userdata) noexcept
		{
			m_callback_interrupt = callback;
			m_userdata_ptr = userdata;
		}
	}
}
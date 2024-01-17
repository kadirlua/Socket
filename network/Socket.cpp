// MIT License

// Copyright (c) 2021-2024 kadirlua

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

namespace sdk {

	namespace {
		constexpr auto const DEFAULT_TIMEOUT = 1000L;
	}

	namespace network {

		bool Socket::m_wsaInit = false;

		bool Socket::WSAInit(unsigned short versionReq) noexcept
		{
			// There's no harm in calling WSAStartup() multiple times but no benefit
			// either, we may as well skip it after the first.
			if (!m_wsaInit) {
#ifdef _WIN32
				WSADATA wsaData;
				if (WSAStartup(versionReq, &wsaData) != 0) {
					return false;
				}
#endif
				m_wsaInit = true;
			}
			return true;
		}

		void Socket::WSADeinit() noexcept
		{
#ifdef _WIN32
			if (m_wsaInit) {
				WSACleanup();
				m_wsaInit = false;
			}
#endif
		}

		Socket::Socket(int portNumber, ProtocolType type /*= protocol_type::tcp*/, IpVersion ipVer /*= IpVersion::IPv4*/) :
			m_portNumber{ portNumber },
			m_protocolType{ type },
			m_ipVersion{ ipVer }
		{
			m_socketId = socket(static_cast<int>(m_ipVersion), static_cast<int>(type), 0);
			if (m_socketId == INVALID_SOCKET) {
				throw general::SocketException(WSAGetLastError());
			}

			if (m_ipVersion == IpVersion::IPv4) {
				m_sockAddressIpv4.sin_family = AF_INET;
				m_sockAddressIpv4.sin_addr.s_addr = htonl(INADDR_ANY);
				m_sockAddressIpv4.sin_port = htons(m_portNumber);
			}
			else if (m_ipVersion == IpVersion::IPv6) {
				m_sockAddressIpv6.sin6_family = AF_INET6;
				m_sockAddressIpv6.sin6_port = htons(m_portNumber);
				m_sockAddressIpv6.sin6_addr = in6addr_any;
			}
		}

		Socket::~Socket()
		{
			if (m_socketId != INVALID_SOCKET) {
				shutdown(m_socketId, SD_BOTH);
				while (closesocket(m_socketId) == SOCKET_ERROR) {
					auto err = WSAGetLastError();
					if (err != WSAEWOULDBLOCK) {
						break;
					}
				}
			}
		}

		void Socket::fillAddrInfo()
		{
			struct addrinfo hints{};
			struct addrinfo *res = nullptr;
			struct addrinfo *ptr = nullptr;

			std::memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST;

			if (getaddrinfo(m_ipAddress.c_str(), std::to_string(m_portNumber).c_str(), &hints, &res) != 0) {
				throw general::SocketException(WSAGetLastError());
			}

			const std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> pResPtr{ res, freeaddrinfo }; 

			for (ptr = pResPtr.get(); ptr != nullptr; ptr = ptr->ai_next) {
				// get the pointer to the address itself,
				// different fields in IPv4 and IPv6:
				if (ptr->ai_family == AF_INET) { // IPv4
					const auto* ipv4 = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
					std::memcpy(&m_sockAddressIpv4.sin_addr, &ipv4->sin_addr, ptr->ai_addrlen);
				}
				else { // IPv6
					const auto* ipv6 = reinterpret_cast<struct sockaddr_in6*>(ptr->ai_addr);
					std::memcpy(&m_sockAddressIpv6.sin6_addr, &ipv6->sin6_addr, ptr->ai_addrlen);
				}
			}
		}

		void Socket::connect()
		{
#ifdef _WIN32
			const struct timeval timeout{ 0, DEFAULT_TIMEOUT };
#else
			struct timeval timeout{ 0, DEFAULT_TIMEOUT };
#endif
			int err{};

			fillAddrInfo();

			const int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_sockAddressIpv4) : sizeof(m_sockAddressIpv6));

			const auto* stAddress = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<const sockaddr*>(&m_sockAddressIpv4) :
				reinterpret_cast<const sockaddr*>(&m_sockAddressIpv6));

			while (::connect(m_socketId, stAddress, addressSize) == SOCKET_ERROR) {
				//	check if any interrupt happened by user
				if (m_callbackInterrupt && m_callbackInterrupt(m_userdataPtr)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				const int lastError = WSAGetLastError();

				switch (lastError) {
				case WSAEWOULDBLOCK: {
					fd_set writeFds{};
					fd_set exceptFds{};

					do {
						if (m_callbackInterrupt && m_callbackInterrupt(m_userdataPtr)) {
							throw general::SocketException(INTERRUPT_MSG);
						}

						FD_ZERO(&writeFds);
						FD_SET(m_socketId, &writeFds);
						FD_ZERO(&exceptFds);
						FD_SET(m_socketId, &exceptFds);
						err = select((int)m_socketId + 1, nullptr, &writeFds, &exceptFds, &timeout);
						if (err < 0) {
							throw general::SocketException(WSAGetLastError());
						}
						if (FD_ISSET(m_socketId, &exceptFds)) {
							throw general::SocketException("Cannot connect to the server");
						}
					} while (!FD_ISSET(m_socketId, &writeFds));

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
			const int addressSize = (m_ipVersion == IpVersion::IPv4 ? sizeof(m_sockAddressIpv4) : sizeof(m_sockAddressIpv6));
			const auto* stAddress = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<const sockaddr*>(&m_sockAddressIpv4) :
				reinterpret_cast<const sockaddr*>(&m_sockAddressIpv6));
			if (::bind(m_socketId, stAddress, addressSize) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		void Socket::listen(int listenCount) const
		{
			if (m_protocolType != ProtocolType::udp) {
				if (::listen(m_socketId, listenCount) == SOCKET_ERROR) {
					throw general::SocketException(WSAGetLastError());
				}
			}
		}

		SOCKET Socket::accept()
		{
			if (m_protocolType != ProtocolType::udp) {
#ifdef _WIN32
				const struct timeval timeout{ 0, DEFAULT_TIMEOUT };
#else
				struct timeval timeout{ 0, DEFAULT_TIMEOUT };
#endif
				socklen_t addrLen = m_ipVersion == IpVersion::IPv4 ? sizeof(m_sockAddressIpv4) : sizeof(m_sockAddressIpv6);
				SOCKET newSockId{};

				auto* stAddress = (m_ipVersion == IpVersion::IPv4 ? reinterpret_cast<sockaddr*>(&m_sockAddressIpv4) :
					reinterpret_cast<sockaddr*>(&m_sockAddressIpv6));

				while ((newSockId = ::accept(m_socketId, stAddress, &addrLen)) == INVALID_SOCKET) {
					//	check if any interrupt happened by user
					if (m_callbackInterrupt && m_callbackInterrupt(m_userdataPtr)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (auto lasterror = WSAGetLastError()) {
					case WSAEWOULDBLOCK: {
						fd_set readFds{};
						fd_set exceptFds{};

						do {
							if (m_callbackInterrupt && m_callbackInterrupt(m_userdataPtr)) {
								throw general::SocketException(INTERRUPT_MSG);
							}

							FD_ZERO(&readFds);
							FD_SET(m_socketId, &readFds);
							FD_ZERO(&exceptFds);
							FD_SET(m_socketId, &exceptFds);
							auto err = select((int)m_socketId + 1, &readFds, nullptr, &exceptFds, &timeout);
							if (err < 0) {
								throw general::SocketException(WSAGetLastError());
							}
							if (FD_ISSET(m_socketId, &exceptFds)) {
								throw general::SocketException("Cannot connect to the server");
							}
						} while (!FD_ISSET(m_socketId, &readFds));
					} break;
					default:
						throw general::SocketException(lasterror);
					}
				}

				return newSockId;
			}

			return 0;
		}

		std::shared_ptr<SocketDescriptor> Socket::createNewSocket(SOCKET socketId) const
		{
			return std::make_shared<SocketDescriptor>(socketId, *this);
		}

		void Socket::setInterruptCallback(const socket_interrupt_callback_t& callback, void* userdata) noexcept
		{
			m_callbackInterrupt = callback;
			m_userdataPtr = userdata;
		}
	}
}
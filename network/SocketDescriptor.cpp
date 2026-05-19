// MIT License

// Copyright (c) 2021-2026 kadirlua

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

#include "SocketDescriptor.h"

#include "Socket.h"
#include "SocketException.h"
#include "SocketOption.h"

#include <array>
#include <cstring>
#include <iterator>
#include <memory>

namespace sdk {
	namespace network {

		namespace {

			constexpr const auto DEFAULT_RECV_TIMEOUT = 5L;

			std::string EndpointToString(const sockaddr_storage& endpoint, int& portNumber)
			{
				std::array<char, INET6_ADDRSTRLEN> address{};

				if (endpoint.ss_family == AF_INET) {
					const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&endpoint);
					portNumber = ntohs(ipv4->sin_port);

					if (inet_ntop(AF_INET, &ipv4->sin_addr, address.data(), static_cast<socklen_t>(address.size())) == nullptr) {
						throw general::SocketException(WSAGetLastError());
					}
				}
				else if (endpoint.ss_family == AF_INET6) {
					const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(&endpoint);
					portNumber = ntohs(ipv6->sin6_port);

					if (inet_ntop(AF_INET6, &ipv6->sin6_addr, address.data(), static_cast<socklen_t>(address.size())) == nullptr) {
						throw general::SocketException(WSAGetLastError());
					}
				}
				else {
					throw general::SocketException("Unsupported endpoint address family");
				}

				return address.data();
			}

			int SendAll(SOCKET socketId,
			            const char* data,
			            int dataSize,
			            const socketInterruptCallback& callbackInterrupt,
			            const Socket& socketRef)
			{
				int totalSent = 0;

				while (totalSent < dataSize) {
					const auto sentBytes = send(socketId, data + totalSent, dataSize - totalSent, 0);

					if (sentBytes > 0) {
						totalSent += sentBytes;
						continue;
					}

					if (callbackInterrupt && callbackInterrupt(socketRef)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (const auto lastError = WSAGetLastError()) {
					case WSAEWOULDBLOCK:
						break;

					default:
						throw general::SocketException(lastError);
					}
				}

				return totalSent;
			}

		}

		SocketDescriptor::SocketDescriptor(SOCKET socketId, const Socket& socketRef) noexcept :
			m_socketId{ socketId },
			m_socketRef{ socketRef }
		{
		}

		SocketDescriptor::~SocketDescriptor()
		{
			if (m_socketId != INVALID_SOCKET) {
				shutdown(m_socketId, SD_BOTH);

				while (closesocket(m_socketId) == SOCKET_ERROR) {
					const auto err = WSAGetLastError();

					if (err != WSAEWOULDBLOCK) {
						break;
					}
				}
			}
		}

		std::string SocketDescriptor::read(int maxSize /*= 0*/) const
		{
			const int bufLen = (maxSize > 0 && maxSize < MAX_MESSAGE_SIZE) ? maxSize : MAX_MESSAGE_SIZE - 1;

			std::string strMessage;
			std::vector<char> dataVec(bufLen);
			int receiveByte{};
			int iResult{};

#ifdef _WIN32
			constexpr struct timeval tVal = { 0, 0 };
#else
			struct timeval tVal = { 0, 0 };
#endif

			fd_set readFds{};
			fd_set exceptFds{};

			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;
			const SocketOption<SocketDescriptor> socketOpt{ *this };

			while (true) {
				while ((receiveByte = recv(m_socketId, dataVec.data(), bufLen, 0)) == SOCKET_ERROR) {
					if (callbackInterrupt && callbackInterrupt(m_socketRef)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (const auto lasterror = WSAGetLastError()) {
					case WSAEWOULDBLOCK: {
						auto recvTimeout = socketOpt.getRecvTimeout();

						// The default value of receive timeout is 0.
						// If a user decided to set timeout value, there is no problem at all.
						// Otherwise, set the default value to an acceptable timeout value.
						if (recvTimeout.tv_sec == 0 && recvTimeout.tv_usec == 0) {
							recvTimeout.tv_sec = DEFAULT_RECV_TIMEOUT;
						}

						FD_ZERO(&readFds);
						FD_SET(m_socketId, &readFds);

						iResult = select(static_cast<int>(m_socketId) + 1, &readFds, nullptr, nullptr, &recvTimeout);

						if (iResult < 0) {
							throw general::SocketException(WSAGetLastError());
						}

						if (!FD_ISSET(m_socketId, &readFds)) {
							return strMessage;
						}
					} break;

					default:
						throw general::SocketException(lasterror);
					}
				}

				if (receiveByte == 0) {
					return strMessage; // the connection is closed.
				}

				if (receiveByte > 0) {
					std::move(dataVec.begin(), dataVec.begin() + receiveByte, std::back_inserter(strMessage));
				}

				if (callbackInterrupt && callbackInterrupt(m_socketRef)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				if (maxSize > 0 && strMessage.size() >= static_cast<std::size_t>(maxSize)) {
					break;
				}

				/*
				 * Wait for a while to detect if we reached the end of the data.
				 * select will return immediately without waiting.
				 */
				FD_ZERO(&readFds);
				FD_SET(m_socketId, &readFds);

				FD_ZERO(&exceptFds);
				FD_SET(m_socketId, &exceptFds);

				iResult = select(static_cast<int>(m_socketId) + 1, &readFds, nullptr, &exceptFds, &tVal);

				if (iResult < 0) {
					throw general::SocketException(WSAGetLastError());
				}

				if (!FD_ISSET(m_socketId, &readFds) || FD_ISSET(m_socketId, &exceptFds)) {
					break;
				}

				if (receiveByte <= 0) {
					break;
				}
			}

			return strMessage;
		}

		std::size_t SocketDescriptor::read(char& msgByte) const
		{
			int const numBytes = recv(m_socketId, &msgByte, 1, 0);

			if (numBytes < 0) {
				throw general::SocketException(WSAGetLastError());
			}

			return static_cast<std::size_t>(numBytes);
		}

		std::size_t SocketDescriptor::read(std::vector<unsigned char>& message, int maxSize /*= 0*/) const
		{
			const auto receivedStr = read(maxSize);
			std::move(receivedStr.begin(), receivedStr.end(), std::back_inserter(message));

			return message.size();
		}

		std::size_t SocketDescriptor::read(std::string& message, int maxSize /*= 0*/) const
		{
			message = read(maxSize);

			return message.size();
		}

		int SocketDescriptor::write(std::initializer_list<char> dataList)
		{
			return write(dataList.begin(), static_cast<int>(dataList.size()));
		}

		int SocketDescriptor::write(const char* data, int dataSize)
		{
			if (data == nullptr || dataSize <= 0) {
				return 0;
			}

			return SendAll(m_socketId, data, dataSize, m_socketRef.m_callbackInterrupt, m_socketRef);
		}

		int SocketDescriptor::write(const std::vector<unsigned char>& message)
		{
			if (message.empty()) {
				return 0;
			}

			return write(reinterpret_cast<const char*>(message.data()), static_cast<int>(message.size()));
		}

		int SocketDescriptor::write(const std::string& message)
		{
			return write(message.c_str(), static_cast<int>(message.size()));
		}

		std::size_t SocketDescriptor::readFrom(std::vector<unsigned char>& message,
		                                       std::string& ipAddress,
		                                       int& portNumber,
		                                       int maxSize /*= 0*/) const
		{
			const int bufLen = (maxSize > 0 && maxSize < MAX_MESSAGE_SIZE) ? maxSize : MAX_MESSAGE_SIZE - 1;
			std::vector<unsigned char> dataVec(static_cast<std::size_t>(bufLen));

			sockaddr_storage endpoint{};
			socklen_t endpointLen = sizeof(endpoint);

			const auto receivedBytes = recvfrom(m_socketId,
			                                    reinterpret_cast<char*>(dataVec.data()),
			                                    bufLen,
			                                    0,
			                                    reinterpret_cast<sockaddr*>(&endpoint),
			                                    &endpointLen);

			if (receivedBytes < 0) {
				throw general::SocketException(WSAGetLastError());
			}

			if (receivedBytes == 0) {
				return 0;
			}

			ipAddress = EndpointToString(endpoint, portNumber);

			const auto oldSize = message.size();
			message.insert(message.end(), dataVec.begin(), dataVec.begin() + receivedBytes);

			return message.size() - oldSize;
		}

		std::size_t SocketDescriptor::readFrom(std::string& message,
		                                       std::string& ipAddress,
		                                       int& portNumber,
		                                       int maxSize /*= 0*/) const
		{
			std::vector<unsigned char> dataVec;
			const auto receivedBytes = readFrom(dataVec, ipAddress, portNumber, maxSize);

			message.assign(dataVec.begin(), dataVec.end());

			return receivedBytes;
		}

		int SocketDescriptor::writeTo(const char* data, int dataSize, const std::string& ipAddress, int portNumber)
		{
			if (data == nullptr || dataSize <= 0) {
				return 0;
			}

			addrinfo hints{};
			addrinfo* result = nullptr;

			std::memset(&hints, 0, sizeof(hints));
			hints.ai_family = static_cast<int>(m_socketRef.m_ipVersion);
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;

			const auto ret = getaddrinfo(ipAddress.c_str(), std::to_string(portNumber).c_str(), &hints, &result);

			if (ret != 0) {
#ifdef _WIN32
				throw general::SocketException(WSAGetLastError());
#else
				throw general::SocketException(gai_strerror(ret));
#endif
			}

			const std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resultPtr{ result, freeaddrinfo };

			const auto sentBytes = sendto(m_socketId,
			                              data,
			                              dataSize,
			                              0,
			                              resultPtr->ai_addr,
			                              static_cast<int>(resultPtr->ai_addrlen));

			if (sentBytes == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}

			return sentBytes;
		}

		int SocketDescriptor::writeTo(const std::vector<unsigned char>& message, const std::string& ipAddress, int portNumber)
		{
			if (message.empty()) {
				return 0;
			}

			return writeTo(reinterpret_cast<const char*>(message.data()), static_cast<int>(message.size()), ipAddress, portNumber);
		}

		int SocketDescriptor::writeTo(const std::string& message, const std::string& ipAddress, int portNumber)
		{
			return writeTo(message.c_str(), static_cast<int>(message.size()), ipAddress, portNumber);
		}

	}
}

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

#include "SocketDescriptor.h"
#include "Socket.h"
#include "general/SocketException.h"
#include "SocketOption.h"

#include <memory>
#include <iterator>

namespace sdk {
	namespace network {

		namespace {
			constexpr const auto DEFAULT_RECV_TIMEOUT = 5L;
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
			const struct timeval tVal = { 0, 0 };
#else
			struct timeval tVal = { 0, 0 };
#endif
			fd_set readFds{};
			fd_set exceptFds{};

			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			const SocketOption<SocketDescriptor> socketOpt{ *this };

			/*if (socketOpt.getBytesAvailable() == 0)
				return strMessage;*/

			do {
				while ((receiveByte = recv(m_socketId, dataVec.data(), bufLen, 0)) == SOCKET_ERROR) {
					if (callbackInterrupt &&
						callbackInterrupt(m_socketRef)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (const auto lasterror = WSAGetLastError()) {
					case WSAEWOULDBLOCK: {
						auto recvTimeout = socketOpt.getRecvTimeout();
						//	The default value of recieve timeout is 0.
						//	If a user decided to set timeout value, there is no problem at all.
						//	Otherwise, set the default value to an acceptable timeout value.
						if (recvTimeout.tv_sec == 0 &&
							recvTimeout.tv_usec == 0) {
							recvTimeout.tv_sec = DEFAULT_RECV_TIMEOUT;
						}

						FD_ZERO(&readFds);
						FD_SET(m_socketId, &readFds);

						iResult = select((int)m_socketId + 1, &readFds, nullptr, nullptr, &recvTimeout);
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
					std::move(dataVec.begin(), dataVec.begin() + receiveByte,
						std::back_inserter(strMessage));
				}

				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				if (maxSize > 0 && strMessage.size() >= (std::size_t)maxSize) {
					break;
				}

				/*
				 *	Wait for a while to detect if we reached the end of the data
				 *	select will return immediately without waiting...
				 */

				FD_ZERO(&readFds);
				FD_SET(m_socketId, &readFds);
				FD_ZERO(&exceptFds);
				FD_SET(m_socketId, &exceptFds);
				iResult = select((int)m_socketId + 1, &readFds, nullptr, &exceptFds, &tVal);
				if (iResult < 0) {
					throw general::SocketException(WSAGetLastError());
				}
				if (!FD_ISSET(m_socketId, &readFds) || FD_ISSET(m_socketId, &exceptFds)) {
					break;
				}
			} while (receiveByte > 0);

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

		int SocketDescriptor::write(std::initializer_list<char> dataList) const
		{
			return write(dataList.begin(), static_cast<int>(dataList.size()));
		}

		int SocketDescriptor::write(const char* data, int dataSize) const
		{
			int sendBytes = 0;

			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			while ((sendBytes = send(m_socketId, data, dataSize, 0)) == SOCKET_ERROR) {
				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				switch (const auto lasterror = WSAGetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				default:
					throw general::SocketException(lasterror);
				}
			}

			return sendBytes;
		}

		int SocketDescriptor::write(const std::vector<unsigned char>& message) const
		{
			const std::string strBuf{ message.begin(), message.end() };
			return write(strBuf.c_str(), static_cast<int>(strBuf.size()));
		}

		int SocketDescriptor::write(const std::string& message) const
		{
			return write(message.c_str(), static_cast<int>(message.size()));
		}
	}
}
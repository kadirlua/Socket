// MIT License

// Copyright (c) 2021-2025 kadirlua

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

#ifndef SOCKET_DESCRIPTOR_H
#define SOCKET_DESCRIPTOR_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef UINT_PTR
#if defined(_WIN64)
typedef unsigned long long UINT_PTR;
#else
typedef unsigned int UINT_PTR;
#endif

typedef UINT_PTR SOCKET;
#endif

#ifndef closesocket
#define closesocket close
#endif

#ifndef ioctlsocket
#define ioctlsocket ioctl
#endif

#define SD_RECEIVE 0x00
#define SD_SEND 0x01
#define SD_BOTH 0x02

#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEALREADY EALREADY
#define WSAEISCONN EISCONN
#define WSAEINPROGRESS EINPROGRESS

#define WSAGetLastError() errno

#endif

#include "SocketExport.h"

#include <vector>
#include <string>

#if (__cplusplus >= 201703L)
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD
#endif

namespace sdk {
	namespace network {

		class Socket; // forward declaration

		/**
		 * @brief This class is used for socket descriptor operations.
		 *	You can read and write operations with this class.
		 */
		class SOCKET_API SocketDescriptor {
		public:
			explicit SocketDescriptor(SOCKET socketId, const Socket& socketRef) noexcept;
			virtual ~SocketDescriptor();

			/**
			 * @brief This function used for reading operations from related socket.
			 * @param msgByte of message.
			 * @return Return byte count that read.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(char& msgByte) const;

			/**
			 * @brief This function used for reading operations from related socket.
			 * @param message the message that you want to read.
			 * @param maxSize maximum size of the message.
			 * @return Return byte count that read.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(std::vector<unsigned char>& message, int maxSize = 0) const;

			/**
			 * @brief This function used for reading operations from related socket.
			 * @param message the message that you want to read.
			 * @param maxSize maximum size of the message.
			 * @return Return byte count that read.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(std::string& message, int maxSize = 0) const;

			/**
			 * @brief This function used for reading operations from related socket.
			 * @param dataList initializer_list of data via modern c++.
			 * @return Return byte count that read.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(std::initializer_list<char> dataList);

			/**
			 * @brief This function used for writing operations from related socket.
			 * @param data Bytes of message.
			 * @param dataSize Size of message.
			 * @return Return byte count that write.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const char* data, int dataSize);

			/**
			 * @brief This function used for writing operations from related socket.
			 * @param message Bytes of vector.
			 * @return Return byte count that write.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const std::vector<unsigned char>& message);

			/**
			 * @brief This function used for writing operations from related socket.
			 * @param message string.
			 * @return Return byte count that write.
			 * @exception this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const std::string& message);

			/**
			 * @brief Gets a socket id from related socket.
			 * @return The id of socket.
			 * @exception This function never throws an exception.
			 */
			NODISCARD SOCKET getSocketId() const noexcept
			{
				return m_socketId;
			}

		protected:
			[[nodiscard]] virtual std::string read(int maxSize = 0) const;

			SOCKET m_socketId{ INVALID_SOCKET };
			static constexpr int MAX_MESSAGE_SIZE = 8096;
			const Socket& m_socketRef;
		};
	}
}

#endif // SOCKET_DESCRIPTOR_H

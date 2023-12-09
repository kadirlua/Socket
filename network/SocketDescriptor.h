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

#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEALREADY EALREADY
#define WSAEISCONN EISCONN

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

		/*
		 *	Creates an instance of socket object via socket id
		 *	to use independent connection operations.
		 */

		class Socket; // forward declaration

		class SOCKET_API SocketDescriptor {
		public:
			explicit SocketDescriptor(SOCKET socketId, const Socket& socket_ref) noexcept;
			virtual ~SocketDescriptor();

			/*
			 *	This function used for reading operations from related socket.
			 *	param1: One byte character to read.
			 *	returns: Return byte count that read.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(char& msgByte) const;

			/*
			 *	This function used for reading operations from related socket.
			 *	param1: Bytes of vector.
			 *	param2: max read size.
			 *	returns: Return byte count that read.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(std::vector<unsigned char>& message, int max_size = 0) const;
			/*
			 *	This function used for reading operations from related socket.
			 *	param1: Bytes.
			 *	param2: max read size.
			 *	returns: Return byte count that read.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual std::size_t read(std::string& message, int max_size = 0) const;

			/*
			 *	This function used for writing operations from related socket.
			 *	param1: initializer_list of data via modern c++.
			 *	returns: Return byte count that write.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(std::initializer_list<char> data_list) const;

			/*
			 *	This function used for writing operations from related socket.
			 *	param1: Bytes of message.
			 *	param2: Size of message.
			 *	returns: Return byte count that write.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const char* data, int data_size) const;
			/*
			 *	This function used for writing operations from related socket.
			 *	param1: Bytes of vector.
			 *	returns: Return byte count that write.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const std::vector<unsigned char>& message) const;
			/*
			 *	This function used for writing operations from related socket.
			 *	param1: string message.
			 *	returns: Return byte count that write.
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual int write(const std::string& message) const;
			/*
			 *	Gets a socket id from related socket.
			 *	returns: The id of socket.
			 *	exception: This function never throws an exception.
			 */
			NODISCARD SOCKET getSocketId() const noexcept
			{
				return m_socket_id;
			}

		protected:
			virtual std::string read(int max_size = 0) const;

			SOCKET m_socket_id{};
			static constexpr int MAX_MESSAGE_SIZE = 8096;
			const Socket& m_socket_ref;
		};
	}
}

#endif	// SOCKET_DESCRIPTOR_H
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

#ifndef SOCKET_OPTION_H
#define SOCKET_OPTION_H

#include "SocketExport.h"

#ifdef _WIN32
#include <WinSock2.h>
#elif __linux__ || defined(__APPLE__)
#include <sys/time.h>
#endif

#include <cstdint>

namespace sdk {
	namespace network {

		enum class SocketOpt : std::uint8_t {
			OFF,
			ON
		};

		template <typename T>
		class SOCKET_API SocketOption final {
		private:
			const T& m_socket;

		public:
			explicit SocketOption(const T& socket) :
				m_socket{ socket }
			{
			}

			//	non-copyable
			SocketOption(const SocketOption&) = delete;
			SocketOption& operator=(const SocketOption&) = delete;

			/*
			 *	Enables or disables debug mode on socket.
			 *	param1: Debug mode is active if 1, disabled 0.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setDebug(SocketOpt debugMode) const;
			/*
			 *	If you want to bind two sockets of the same protocol to the same source address and port.
			 *	param1: Reuse address is active if 1, disabled 0.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setReuseAddr(SocketOpt reuseMode) const;
			/*
			 *	Allow an application to enable keep-alive packets for a socket connection.
			 *	param1: Keep alive is active if 1, disabled 0.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setKeepAlive(SocketOpt keepAliveMode) const;
			/*
			 *	Enables or disables non-blocking mode on socket.
			 *	param1: Non-blocking mode is active if 1, disabled 0.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setBlockingMode(SocketOpt blockingMode) const;
			/*
			 *	Enables or disables linger option on socket.
			 *	param1: Linger option is active if 1, disabled 0.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setLingerOpt(SocketOpt mode, unsigned short second) const;
			/*
			 *	Sets default timeout value for socket connections.
			 *	param1: seconds.
			 *	param2: microseconds.
			 *	returns: nothing.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			void setRecvTimeout(long seconds, long microseconds) const;
			/*
			 *	Gets linger option state on socket.
			 *	returns: Active if returns 1, disabled 0.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			int getLingerOpt() const;
			/*
			 *	Gets debug mode state on socket.
			 *	returns: Active if returns 1, disabled 0.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			int getDebug() const;
			/*
			 *	Gets reuse address state on socket.
			 *	returns: Active if returns 1, disabled 0.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			int getReuseAddr() const;
			/*
			 *	Gets keep alive state on socket.
			 *	returns: Active if returns 1, disabled 0.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			int getKeepAlive() const;

			/*
			 *	Gets default receive timeout.
			 *	returns: returns default receive timeout.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			timeval getRecvTimeout() const;

			/*
			 *	Gets default receive timeout.
			 *	returns: returns default receive timeout.
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			unsigned long getBytesAvailable() const;
		};
	}
}

#endif	// SOCKET_OPTION_H
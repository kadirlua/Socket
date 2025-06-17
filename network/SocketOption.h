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

		/**
		 * @class SocketOption
		 * @brief This class is a helper class for setting and getting socket options.
		 */
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

			/**
			 * @brief Enables or disables debug mode on socket.
			 * @param debugMode Debug mode is active if 1, disabled 0.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setDebug(SocketOpt debugMode);

			/**
			 * @brief If you want to bind two sockets of the same protocol to the same source address and port.
			 * @param reuseMode Reuse address is active if 1, disabled 0.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setReuseAddr(SocketOpt reuseMode);

			/**
			 * @brief Allow an application to enable keep-alive packets for a socket connection.
			 * @param keepAliveMode Keep alive is active if 1, disabled 0.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setKeepAlive(SocketOpt keepAliveMode);

			/**
			 * @brief Enables or disables non-blocking mode on socket.
			 * @param blockingMode Non-blocking mode is active if 1, disabled 0.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setBlockingMode(SocketOpt blockingMode);

			/**
			 * @brief Enables or disables linger option on socket.
			 * @param mode Linger option is active if 1, disabled 0.
			 * @param second Linger option time.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setLingerOpt(SocketOpt mode, unsigned short second);

			/**
			 * @brief Sets default timeout value for socket connections.
			 * @param seconds Timeout value in seconds.
			 * @param microseconds Timeout value in microseconds.
			 * @return nothing.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			void setRecvTimeout(long seconds, long microseconds);

			/**
			 * @brief Gets linger option state on socket.
			 * @return Active if returns 1, disabled 0.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] int getLingerOpt() const;

			/**
			 * @brief Gets debug mode state on socket.
			 * @return Active if returns 1, disabled 0.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] int getDebug() const;

			/**
			 * @brief Gets reuse address state on socket.
			 * @return Active if returns 1, disabled 0.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] int getReuseAddr() const;

			/**
			 * @brief Gets keep alive state on socket.
			 * @return Active if returns 1, disabled 0.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] int getKeepAlive() const;

			/**
			 * @brief Gets default receive timeout.
			 * @return returns default receive timeout.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] timeval getRecvTimeout() const;

			/**
			 * @brief Gets bytes available on socket.
			 * @return returns bytes available on socket.
			 * @exception This function throws an SocketException if an error occurs.
			 */
			[[nodiscard]] unsigned long getBytesAvailable() const;
		};
	}
}

#endif // SOCKET_OPTION_H
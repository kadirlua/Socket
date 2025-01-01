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

#include "SocketOption.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "SocketException.h"

namespace sdk {
	namespace network {

		template <typename T>
		void SocketOption<T>::setDebug(SocketOpt debugMode)
		{
			const auto mode = static_cast<int>(debugMode);
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG,
					reinterpret_cast<const char*>(&mode), sizeof(mode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setReuseAddr(SocketOpt reuseMode)
		{
			const auto mode = static_cast<int>(reuseMode);
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR,
					reinterpret_cast<const char*>(&mode), sizeof(mode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setKeepAlive(SocketOpt keepAliveMode)
		{
			const auto mode = static_cast<int>(keepAliveMode);
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE,
					reinterpret_cast<const char*>(&mode), sizeof(mode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setBlockingMode(SocketOpt blockingMode)
		{
			auto mode = static_cast<unsigned long>(blockingMode);
			if (ioctlsocket(m_socket.getSocketId(), FIONBIO, &mode) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setLingerOpt(SocketOpt mode, unsigned short second)
		{
			const struct linger opt{ static_cast<unsigned short>(mode), second };
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER,
					reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setRecvTimeout(long seconds, long microseconds)
		{
#if defined(__APPLE__)
			const struct timeval tVal{ seconds, static_cast<__darwin_suseconds_t>(microseconds) };
#else
			const struct timeval tVal{ seconds, microseconds };
#endif
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO,
					reinterpret_cast<const char*>(&tVal), sizeof(tVal)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		int SocketOption<T>::getLingerOpt() const
		{
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER,
					reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
			return myOption;
		}

		template <typename T>
		int SocketOption<T>::getDebug() const
		{
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG,
					reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
			return myOption;
		}

		template <typename T>
		int SocketOption<T>::getReuseAddr() const
		{
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR,
					reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}

			return myOption;
		}

		template <typename T>
		int SocketOption<T>::getKeepAlive() const
		{
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE,
					reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
			return myOption;
		}

		template <typename T>
		timeval SocketOption<T>::getRecvTimeout() const
		{
			struct timeval recvTimeout{};
			socklen_t myOptionLen = sizeof(recvTimeout);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO,
					reinterpret_cast<char*>(&recvTimeout), &myOptionLen) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
			return recvTimeout;
		}

		template <typename T>
		unsigned long SocketOption<T>::getBytesAvailable() const
		{
			u_long bufSize = 0;
			if (ioctlsocket(m_socket.getSocketId(), FIONREAD, &bufSize) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}

			return bufSize;
		}

		// Explicit template specialization
		template class SocketOption<Socket>;
		template class SocketOption<SocketDescriptor>;
#if OPENSSL_SUPPORTED
		template class SocketOption<SSLSocket>;
		template class SocketOption<SSLSocketDescriptor>;
#endif // OPENSSL_SUPPORTED
	}
}
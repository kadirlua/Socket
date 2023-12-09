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

#include "SocketOption.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "general/SocketException.h"

namespace sdk {
	namespace network {

		template <typename T>
		void SocketOption<T>::setDebug(int debugMode) const
		{
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG,
					reinterpret_cast<const char*>(&debugMode), sizeof(debugMode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setReuseAddr(int reuseMode) const
		{
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR,
					reinterpret_cast<const char*>(&reuseMode), sizeof(reuseMode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setKeepAlive(int keepAliveMode) const
		{
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE,
					reinterpret_cast<const char*>(&keepAliveMode), sizeof(keepAliveMode)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setBlockingMode(unsigned long blockingMode) const
		{
			if (ioctlsocket(m_socket.getSocketId(), FIONBIO, &blockingMode) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setLingerOpt(unsigned short mode, unsigned short second) const
		{
			const linger opt{ mode, second };
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER,
					reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setRecvTimeout(long seconds, long microseconds) const
		{
#if defined(__APPLE__)
			const timeval tVal{ seconds, static_cast<__darwin_suseconds_t>(microseconds) };
#else
			const timeval tVal{ seconds, microseconds };
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
			timeval recvTimeout{};
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
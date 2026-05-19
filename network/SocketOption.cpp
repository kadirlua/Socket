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

#include "SocketOption.h"

#include "Socket.h"
#include "SocketException.h"
#include "SSLSocket.h"

namespace sdk {
	namespace network {

		namespace {

#ifdef _WIN32

			DWORD MakeTimeout(long seconds, long microseconds)
			{
				return static_cast<DWORD>((seconds * 1000L) + (microseconds / 1000L));
			}

			timeval ToTimeval(DWORD timeoutMs)
			{
				timeval timeout{};
				timeout.tv_sec = static_cast<long>(timeoutMs / 1000U);
				timeout.tv_usec = static_cast<long>((timeoutMs % 1000U) * 1000U);

				return timeout;
			}

#else

			timeval MakeTimeout(long seconds, long microseconds)
			{
#if defined(__APPLE__)
				return timeval{ seconds, static_cast<__darwin_suseconds_t>(microseconds) };
#else
				return timeval{ seconds, microseconds };
#endif
			}

#endif

			template <typename OptionT, typename LengthT>
			void SetSocketOpt(SOCKET socketId, int level, int option, const OptionT& value, LengthT length)
			{
				if (setsockopt(socketId, level, option, reinterpret_cast<const char*>(&value), length) == SOCKET_ERROR) {
					throw general::SocketException(WSAGetLastError());
				}
			}

			template <typename OptionT>
			OptionT GetSocketOpt(SOCKET socketId, int level, int option)
			{
				OptionT value{};
				socklen_t valueLen = sizeof(value);

				if (getsockopt(socketId, level, option, reinterpret_cast<char*>(&value), &valueLen) == SOCKET_ERROR) {
					throw general::SocketException(WSAGetLastError());
				}

				return value;
			}

		}

		template <typename T>
		void SocketOption<T>::setDebug(SocketOpt debugMode)
		{
			const auto mode = static_cast<int>(debugMode);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG, mode, sizeof(mode));
		}

		template <typename T>
		void SocketOption<T>::setReuseAddr(SocketOpt reuseMode)
		{
			const auto mode = static_cast<int>(reuseMode);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR, mode, sizeof(mode));
		}

		template <typename T>
		void SocketOption<T>::setKeepAlive(SocketOpt keepAliveMode)
		{
			const auto mode = static_cast<int>(keepAliveMode);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE, mode, sizeof(mode));
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
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER, opt, sizeof(opt));
		}

		template <typename T>
		void SocketOption<T>::setRecvTimeout(long seconds, long microseconds)
		{
			const auto tVal = MakeTimeout(seconds, microseconds);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO, tVal, sizeof(tVal));
		}

		template <typename T>
		void SocketOption<T>::setSendTimeout(long seconds, long microseconds)
		{
			const auto tVal = MakeTimeout(seconds, microseconds);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_SNDTIMEO, tVal, sizeof(tVal));
		}

		template <typename T>
		void SocketOption<T>::setBroadcast(SocketOpt broadcastMode)
		{
			const auto mode = static_cast<int>(broadcastMode);
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_BROADCAST, mode, sizeof(mode));
		}

		template <typename T>
		void SocketOption<T>::setSendBufferSize(int bytes)
		{
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_SNDBUF, bytes, sizeof(bytes));
		}

		template <typename T>
		void SocketOption<T>::setRecvBufferSize(int bytes)
		{
			SetSocketOpt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVBUF, bytes, sizeof(bytes));
		}

		template <typename T>
		int SocketOption<T>::getLingerOpt() const
		{
			const auto lingerOpt = GetSocketOpt<struct linger>(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER);
			return lingerOpt.l_onoff;
		}

		template <typename T>
		int SocketOption<T>::getDebug() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG);
		}

		template <typename T>
		int SocketOption<T>::getReuseAddr() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR);
		}

		template <typename T>
		int SocketOption<T>::getKeepAlive() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE);
		}

		template <typename T>
		timeval SocketOption<T>::getRecvTimeout() const
		{
#ifdef _WIN32
			const auto timeoutMs = GetSocketOpt<DWORD>(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO);
			return ToTimeval(timeoutMs);
#else
			return GetSocketOpt<timeval>(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO);
#endif
		}

		template <typename T>
		timeval SocketOption<T>::getSendTimeout() const
		{
#ifdef _WIN32
			const auto timeoutMs = GetSocketOpt<DWORD>(m_socket.getSocketId(), SOL_SOCKET, SO_SNDTIMEO);
			return ToTimeval(timeoutMs);
#else
			return GetSocketOpt<timeval>(m_socket.getSocketId(), SOL_SOCKET, SO_SNDTIMEO);
#endif
		}

		template <typename T>
		int SocketOption<T>::getBroadcast() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_BROADCAST);
		}

		template <typename T>
		int SocketOption<T>::getSendBufferSize() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_SNDBUF);
		}

		template <typename T>
		int SocketOption<T>::getRecvBufferSize() const
		{
			return GetSocketOpt<int>(m_socket.getSocketId(), SOL_SOCKET, SO_RCVBUF);
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

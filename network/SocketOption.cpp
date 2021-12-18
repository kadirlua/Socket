#include "SocketOption.h"
#include "Socket.h"
#include "SecureSocket.h"
#include "general/SocketException.h"

namespace sdk {
	namespace network {

		//Explicit template specialization
		template class SocketOption<Socket>;
		template class SocketOption<SocketObject>;
#if OPENSSL_SUPPORTED
		template class SocketOption<SecureSocket>;
		template class SocketOption<SecureSocketObj>;
#endif // OPENSSL_SUPPORTED

		template <typename T>
		void SocketOption<T>::setDebug(int debugMode) const {
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_DEBUG,
				reinterpret_cast<const char*>(&debugMode), sizeof(debugMode)) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setReuseAddr(int reuseMode) const {
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_REUSEADDR,
				reinterpret_cast<const char*>(&reuseMode), sizeof(reuseMode)) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setKeepAlive(int keepAliveMode) const {
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE,
				reinterpret_cast<const char*>(&keepAliveMode), sizeof(keepAliveMode)) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setBlockingMode(unsigned long blockingMode) const {
			if (ioctlsocket(m_socket.getSocketId(), FIONBIO, &blockingMode) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setLingerOpt(unsigned short mode, unsigned short second) const
		{
			const linger sl { mode, second };
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER,
				reinterpret_cast<const char*>(&sl), sizeof(sl)) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		void SocketOption<T>::setRecvTimeout(long seconds, long microseconds) const
		{
			const timeval sl { seconds, microseconds };
			if (setsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO,
				reinterpret_cast<const char*>(&sl), sizeof(sl)) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
		}

		template <typename T>
		int SocketOption<T>::getLingerOpt() const
		{
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_LINGER,
				reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR)
			{
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
				reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR)
			{
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
				reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}

			return myOption;
		}

		template <typename T>
		int SocketOption<T>::getKeepAlive() const {
			int myOption = 0;
			socklen_t myOptionLen = sizeof(myOption);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_KEEPALIVE,
				reinterpret_cast<char*>(&myOption), &myOptionLen) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
			return myOption;
		}

		template<typename T>
		timeval SocketOption<T>::getRecvTimeout() const {
			timeval recvTimeout{};
			socklen_t myOptionLen = sizeof(recvTimeout);

			if (getsockopt(m_socket.getSocketId(), SOL_SOCKET, SO_RCVTIMEO,
				reinterpret_cast<char*>(&recvTimeout), &myOptionLen) == SOCKET_ERROR)
			{
				throw general::SocketException(WSAGetLastError());
			}
			return recvTimeout;
		}

		template<typename T>
		unsigned long SocketOption<T>::getBytesAvailable() const {
			u_long bufSize = 0;
			if (ioctlsocket(m_socket.getSocketId(), FIONREAD, &bufSize) == SOCKET_ERROR)
				throw general::SocketException(WSAGetLastError());

			return bufSize;
		}
	}
}
#ifndef SOCKET_OPTION_H_
#define SOCKET_OPTION_H_

#ifdef _WIN32
#include <WinSock2.h>
#elif __linux__
#include <sys/time.h>
#endif

namespace sdk {
	namespace network {
		template <typename T>
		class SocketOption final {
		private:
			const T& m_socket;
		public:

			explicit SocketOption(const T& t) :
				m_socket{ t } {	}

			//	non-copyable
			SocketOption(const SocketOption&) = delete;
			SocketOption& operator=(const SocketOption&) = delete;

			/*
			*	Enables or disables debug mode on socket.
			*	param1: Debug mode is active if 1, disabled 0.
			*	returns: nothing.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			void setDebug(int debugMode) const;
			/*
			*	If you want to bind two sockets of the same protocol to the same source adress and port.
			*	param1: Reuse adress is active if 1, disabled 0.
			*	returns: nothing.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			void setReuseAddr(int reuseMode) const;
			/*
			*	Allow an application to enable keep-alive packets for a socket connection.
			*	param1: Keep alive is active if 1, disabled 0.
			*	returns: nothing.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			void setKeepAlive(int keepAliveMode) const;
			/*
			*	Enables or disables non-blocking mode on socket.
			*	param1: Non-blocking mode is active if 1, disabled 0.
			*	returns: nothing.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			void setBlockingMode(unsigned long blockingMode) const;
			/*
			*	Enables or disables linger option on socket.
			*	param1: Linger option is active if 1, disabled 0.
			*	returns: nothing.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			void setLingerOpt(unsigned short mode, unsigned short second) const;
			/*
			*	Enables or disables linger option on socket.
			*	param1: Linger option is active if 1, disabled 0.
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
			*	Gets reuse adress state on socket.
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
			*	Gets default recieve timeout.
			*	returns: returns default recieve timeout.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			timeval getRecvTimeout() const;

			/*
			*	Gets default recieve timeout.
			*	returns: returns default recieve timeout.
			*	exception: This function throws an SocketException if an error occurs.
			*/
			unsigned long getBytesAvailable() const;
		};
	}
}

#endif
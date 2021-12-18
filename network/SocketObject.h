#ifndef SOCKET_OBJECT_H_
#define SOCKET_OBJECT_H_

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __linux__
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
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

#define SD_RECEIVE      0x00
#define SD_SEND         0x01

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define WSAEWOULDBLOCK  EWOULDBLOCK
#define WSAEALREADY		EALREADY
#define WSAEISCONN		EISCONN

#define WSAGetLastError() errno

#endif

#include <vector>
#include <string>

namespace sdk {
	namespace network {

		/*
		*	Creates an instance of socket object via socket id
		*	to use independent connection operations.
		*/

		class Socket;	//incomplete type

		class SocketObject
		{
		public:
			SocketObject(SOCKET socketid, const Socket& socket_ref) noexcept;
			virtual ~SocketObject();

			/*
			*	This function used for reading operations from related socket.
			*	param1: One byte character to read.
			*	returns: Return byte count that read.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual size_t read(char& msgByte) const;

			/*
			*	This function used for reading operations from related socket.
			*	param1: Bytes of vector.
			*	param2: max read size.
			*	returns: Return byte count that read.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual size_t read(std::vector<unsigned char>& message, int max_size = 0) const;
			/*
			*	This function used for reading operations from related socket.
			*	param1: Bytes.
			*	param2: max read size.
			*	returns: Return byte count that read.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual size_t read(std::string& message, int max_size = 0) const;

			/*
			*	This function used for writing operations from related socket.
			*	param1: initializer_list of data via modern c++.
			*	returns: Return byte count that write.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual int write(std::initializer_list<char> data_list) const;

			/*
			*	This function used for writing operations from related socket.
			*	param1: Bytes of message.
			*	param2: Size of message.
			*	returns: Return byte count that write.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual int write(const char* data, int data_size) const;
			/*
			*	This function used for writing operations from related socket.
			*	param1: Bytes of vector.
			*	returns: Return byte count that write.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual int write(const std::vector<unsigned char>& message) const;
			/*
			*	This function used for writing operations from related socket.
			*	param1: string message.
			*	returns: Return byte count that write.
			*	exception: this function throws an SocketException if an error occurs.
			*/
			virtual int write(const std::string& message) const;
			/*
			 *	Gets an socket id from related socket.
			 *	returns: The id of socket.
			 *	exception: This function never throws an exception.
			*/
			SOCKET getSocketId() const noexcept {
				return m_socket_id;
			}

		protected:
			virtual std::string read(int max_size = 0) const;

		protected:
			SOCKET m_socket_id{};
			static constexpr int MAX_MESSAGE_SIZE = 8096;
			const Socket& m_socket_ref;
		};
	}
}

#endif
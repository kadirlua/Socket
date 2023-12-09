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

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <memory>
#include <functional>
#include "SocketDescriptor.h"
#include "version.h"

namespace sdk {
	namespace network {

#define SOCKET_VERSION_MAJOR 1
#define SOKCET_VERSION_MINOR 0
#define SOCKET_VERSION_PATCH 0

		enum class ConnMethod {
			client = 1,
			server
		};

		enum class ProtocolType {
			tcp = SOCK_STREAM,
			udp = SOCK_DGRAM
		};

		enum class IpVersion {
			IPv4 = AF_INET,
			IPv6 = AF_INET6
		};

		namespace {
			constexpr auto const INTERRUPT_MSG = "I/O interrupt callback is called by user.";
		}

		//	socket interrupt callback
		using socket_interrupt_callback_t = std::function<bool(void*)>;

		//	This class manage some connection issues for unsecure connection via TCP or UDP.
		//	This class has methods that works blocking or non-blocking mode.
		//	Default mode is blocking mode, if you want to work on non-blocking mode
		//	call setBlockingMode(1) instead.
		//	Before call these methods you must call WSAInit() once.
		//	After all operations done, do not forget to call WSADeinit to clean memory properly.

		class SOCKET_API Socket {
			friend class SocketDescriptor;
			friend class SSLSocketDescriptor;

		public:
			explicit Socket(int portNumber, ProtocolType type = ProtocolType::tcp,
				IpVersion ipVer = IpVersion::IPv4);

			virtual ~Socket();

			// non copyable
			Socket(const Socket&) = delete;
			Socket& operator=(const Socket&) = delete;
			Socket(Socket&&) noexcept = delete;
			Socket& operator=(Socket&&) noexcept = delete;

			/*
			 *	This function initiates use of the Winsock DLL by a process.
			 *	param1: version number
			 *	returns: true if successfully, false otherwise.
			 */
			static bool WSAInit(unsigned short versionReq) noexcept;
			/*
			 *	This function terminates use of the Winsock DLL.
			 *	returns: nothing.
			 */
			static void WSADeinit() noexcept;
			/*
			 *	Duty of connect method is connected to server for client applications.
			 *	returns: nothing
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			virtual void connect();
			/*
			 *	The bind function associates a local address with a socket.
			 *	returns: nothing
			 *	exception: this function throws an SocketException if an error occurs.
			 */
			virtual void bind();
			/*
			 *	The listen function places a socket in a state in which it is listening for an incoming connection.
			 *	This function is useless for udp connections.
			 *	returns: nothing
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			virtual void listen(int listenCount) const;
			/*
			 *	The accept function permits an incoming connection attempt on a socket.
			 *	This function is useless for udp connections.
			 *	returns: nothing
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual SOCKET accept();
			/*
			 *	This function creates an instance of socket.
			 *	param1: The id of socket.
			 *	returns: A shared pointer of socket object.
			 */
			NODISCARD std::shared_ptr<SocketDescriptor> createNewSocket(SOCKET) const;
			/*
			 *	This function is useful for client applications to set an ip address.
			 *	param: Ip address.
			 *	returns: nothing.
			 *	exception: This function never throws an exception.
			 */
			void setIpAddress(std::string ipAddress) noexcept
			{
				m_ipAddress = std::move(ipAddress);
			}
			/*
			 *	This function is useful for all socket applications to set a port number.
			 *	param: Port number.
			 *	returns: nothing.
			 *	exception: This function never throws an exception.
			 */
			void setPortNumber(int portNumber) noexcept
			{
				m_portNumber = portNumber;
			}
			/*
			 *	This function returns socket id if you need.
			 *	returns: The id of socket.
			 *	exception: This function never throws an exception.
			 */
			NODISCARD SOCKET getSocketId() const noexcept
			{
				return m_socketId;
			}
			/*
			 *	This function returns port number if you need.
			 *	returns: port number int type.
			 *	exception: This function never throws an exception.
			 */
			NODISCARD int getPort() const noexcept
			{
				return m_portNumber;
			}

			void setInterruptCallback(const socket_interrupt_callback_t& callback,
				void* userdata) noexcept;

			static const char* getVersionStr() noexcept
			{
				return VERSION_STR(SOCKET_VERSION_MAJOR,
					SOCKET_VERSION_MINOR, SOCKET_VERSION_PATCH);
			}

		protected:
			std::string getIpAddress() const;

		private:
			static bool m_wsaInit;

			socket_interrupt_callback_t m_callbackInterrupt;
			void* m_userdataPtr{};
			int m_portNumber{};
			ProtocolType m_protocolType{ ProtocolType::tcp };
			SOCKET m_socketId{};
			struct sockaddr_in m_sockAddressIpv4{}; // Stores address information.
			struct sockaddr_in6 m_sockAddressIpv6{};
			std::string m_ipAddress{};
			IpVersion m_ipVersion;
		};
	}
}

#endif	// SOCKET_H
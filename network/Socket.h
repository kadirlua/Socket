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

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <memory>
#include <functional>
#include "SocketDescriptor.h"

#include <cstdint>

namespace sdk {
	namespace network {

		constexpr std::uint16_t MakeVer(std::uint8_t lowByte, std::uint8_t highByte) 
		{
			return (lowByte & 0xff) | ((highByte & 0xff) << 8);
		}

		enum class ConnMethod : std::uint8_t {
			client = 1,
			server
		};

		enum class ProtocolType : std::uint8_t {
			tcp = SOCK_STREAM,
			udp = SOCK_DGRAM
		};

		enum class IpVersion : std::uint8_t {
			IPv4 = AF_INET,
			IPv6 = AF_INET6
		};

		// Useful WSA socket DLL versions
		enum {
			WSA_VER_1_0 = MakeVer(1, 0),
			WSA_VER_1_1 = MakeVer(1, 1),
			WSA_VER_2_0 = MakeVer(2, 0),
			WSA_VER_2_1 = MakeVer(2, 1),
			WSA_VER_2_2 = MakeVer(2, 2)
		};

		namespace {
			constexpr auto const INTERRUPT_MSG = "I/O interrupt callback is called by user.";
		}

		class Socket;	// forward declaration

		//	socket interrupt callback
		using socketInterruptCallback = std::function<bool(const Socket&)>;

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
			virtual void listen(int listenCount);
			/*
			 *	The accept function permits an incoming connection attempt on a socket.
			 *	This function is useless for udp connections.
			 *	returns: nothing
			 *	exception: This function throws an SocketException if an error occurs.
			 */
			NODISCARD virtual SOCKET accept();
			/*
			 *	This function creates an instance of socket descriptor.
			 *	param1: The id of socket.
			 *	returns: A shared pointer of socket descriptor.
			 */
			NODISCARD std::shared_ptr<SocketDescriptor> createSocketDescriptor(SOCKET socketId);
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

			void setInterruptCallback(const socketInterruptCallback& callback) noexcept;

		protected:
			void fillAddrInfo();

		private:
			static bool m_wsaInit;

			int m_portNumber{};
			ProtocolType m_protocolType{ ProtocolType::tcp };
			SOCKET m_socketId{ INVALID_SOCKET };
			struct sockaddr_in m_sockAddressIpv4{}; // Stores address information.
			struct sockaddr_in6 m_sockAddressIpv6{};
			socketInterruptCallback m_callbackInterrupt;
			std::string m_ipAddress;
			IpVersion m_ipVersion{ IpVersion::IPv4 };
		};
	}
}

#endif	// SOCKET_H
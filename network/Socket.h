#ifndef SOCKET_H_
#define SOCKET_H_

#include <string>
#include <memory>
#include <functional>
#include "SocketObject.h"
#include "version.h"

namespace sdk {
	namespace network {

#define SOCKET_VERSION_MAJOR 1
#define SOKCET_VERSION_MINOR 0
#define SOCKET_VERSION_PATCH 0

		enum class connection_method {
			client = 1,
			server
		};

		enum class protocol_type {
			tcp = SOCK_STREAM,
			udp = SOCK_DGRAM
		};

		enum class IpVersion {
			IPv4 = AF_INET,
			IPv6 = AF_INET6
		};


		static constexpr auto const INTERRUPT_MSG = "I/O interrupt callback is called by user.";

		//	socket interrupt callback
		using socket_interrupt_callback_t = std::function<bool(void *)>;

		//	This class manage some connection issues for unsecure connection via TCP or UDP.
		//	This class has methods that works blocking or non-blocking mode.
		//	Default mode is blocking mode, if you want to work on non-blocking mode
		//	call setBlockingMode(1) instead.
		//	Before call these methods you must call WSA_startup_init() once.
		//	After all operations done, do not forget to call WSA_Cleanup to clean memory properly.
	
		class Socket {
			friend class SocketObj;
			friend class SecureSocketObj;
		public:
			Socket(int port_, protocol_type type = protocol_type::tcp , IpVersion ipVer = IpVersion::IPv4);
		
			virtual ~Socket();

			//non copyable
			Socket(const Socket&) = delete;
			Socket& operator=(const Socket&) = delete;

			/*
			*	This function initiates use of the Winsock DLL by a process. 
			*	param1: version number
			*	returns: true if succesfully, false otherwise.
			*/
			static bool WSA_startup_init(unsigned short versionReq) noexcept;
			/*
			*	This function terminates use of the Winsock DLL.
			*	returns: nothing.
			*/
			static void WSA_Cleanup() noexcept;
			/*
			*	Duty of connect method is connect to server for client applications.
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
			virtual void listen(int listen_count) const;
			/*
			*	The accept function permits an incoming connection attempt on a socket.
			*	This function is useless for udp connections.
			*	returns: nothing
			*	exception: This function throws an SocketException if an error occurs.
			*/
			virtual SOCKET accept();
			/*
			*	This function creates an instance of socket.
			*	param1: The id of socket.
			*	returns: A shared pointer of socket object.
			*/
			std::shared_ptr<SocketObj> createnewSocket(SOCKET) const;
			/*
			*	This function is useful for client applications to set an ip address.
			*	param: Ip adress.
			*	returns: nothing.
			*	exception: This function never throws an exception.
			*/
			void setIpAddress(std::string ip_) noexcept {
				m_ip_address = ip_;
			}
			/*
			*	This function is useful for all socket applications to set an port numner.
			*	param: Port number.
			*	returns: nothing.
			*	exception: This function never throws an exception.
			*/
			void setPortNumber(int port_) noexcept {
				m_port_number = port_;
			}
			/*
			*	This function returns socket id if you need.
			*	returns: The id of socket.
			*	exception: This function never throws an exception. 
			*/
			SOCKET getSocketId() const noexcept {
				return m_socket_id;
			}
			/*
			*	This function returns port number if you need.
			*	returns: port number int type.
			*	exception: This function never throws an exception.
			*/
			int getPort() const noexcept {
				return m_port_number;
			}

			void setInterruptCallback(const socket_interrupt_callback_t& callback, 
				void * userdata) noexcept;

			static const char* getVersionStr() noexcept {
				return VERSION_STR(SOCKET_VERSION_MAJOR,
					SOCKET_VERSION_MINOR, SOCKET_VERSION_PATCH);
			}

		protected:
			std::string getIpAddress() const;

		private:
			static bool m_wsa_init;

		protected:
			socket_interrupt_callback_t m_callback_interrupt;
			void* m_userdata_ptr{};
			int m_port_number{};
			protocol_type m_protocol_type{ protocol_type::tcp };
			SOCKET m_socket_id{};
			struct sockaddr_in m_st_address_t{};	//Stores address information.
			struct sockaddr_in6 m_st_address6_t{};
			std::string m_ip_address{};
			IpVersion m_ipVersion;
		};
	}
}

#endif
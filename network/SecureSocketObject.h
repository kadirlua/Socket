#ifndef SECURE_SOCKET_OBJECT_H_
#define SECURE_SOCKET_OBJECT_H_

#include "SocketObject.h"

#if OPENSSL_SUPPORTED
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#endif // OPENSSL_SUPPORTED

#include <string>

namespace sdk {
	namespace network {

#if OPENSSL_SUPPORTED

		class SecureSocket;	//incomplete type declaration
		/*
		*	Creates an instance of secure socket layer object via socket id
		*	to use independent connection operations.
		*/
		class SecureSocketObj : public SocketObject
		{
		public:
			SecureSocketObj(SOCKET socketid, const SecureSocket& ss);
			~SecureSocketObj() override;
			/*
			*	This method used for validating hostname for certificate verification.
			*	param1: Hostname such as www.sdk.com.
			*	returns: nothing.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			void setHostname(const char* hostname);
			/*
			*	Duty of connect method is connect to server for client applications using SSL.
			*	returns: nothing
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			void connect();

			/*
			*	This method used for reading operations from related secure socket layer.
			*	param1: One byte character to read.
			*	returns: Return byte count that read.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			size_t read(char& msgByte) const override;
			/*
			*	This method used for reading operations from related secure socket layer.
			*	param1: Bytes of vector.
			*	param2: max read size.
			*	returns: Return byte count that read.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			size_t read(std::vector<unsigned char>& message, int max_size = 0) const override;
			/*
			*	This method used for reading operations from related secure socket layer.
			*	param1: String.
			*	param2: max read size.
			*	returns: Return byte count that read.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			size_t read(std::string& message, int max_size = 0) const override;

			/*
			*	This method used for writing operations from related secure socket layer.
			*	param1: initializer_list of data via modern c++.
			*	returns: Return byte count that write.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			int write(std::initializer_list<char> data_list) const override;

			/*
			*	This method used for writing operations from related secure socket layer.
			*	param1: Bytes of message.
			*	param2: Size of message.
			*	returns: Return byte count that write.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			int write(const char* data, int data_size) const override;
			/*
			*	This method used for writing operations from related secure socket layer.
			*	param1: Bytes of vector message.
			*	param2: Size of message.
			*	returns: Return byte count that write.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			int write(const std::vector<unsigned char>& message) const override;
			/*
			*	This method used for writing operations from related secure socket layer.
			*	param1: String of message.
			*	param2: Size of message.
			*	returns: Return byte count that write.
			*	exception: This method throws an SecureSocketException if an error occurs.
			*/
			int write(const std::string& message) const override;
			/*
			 *	Gets an socket id from related socket.
			 *	returns: The id of socket.
			 *	exception: This function never throws an exception.
			 */
			void accept();
		protected:
			std::string read(int max_size = 0) const override;
		protected:
			std::string m_hostname;
			SSL* m_ssl{};
		};
#endif // OPENSSL_SUPPORTED
	}
}

#endif
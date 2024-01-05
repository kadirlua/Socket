// MIT License

// Copyright (c) 2021-2024 kadirlua

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

#ifndef SSL_SOCKET_DESCRIPTOR_H
#define SSL_SOCKET_DESCRIPTOR_H

#include "SocketDescriptor.h"

#if OPENSSL_SUPPORTED
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#endif // OPENSSL_SUPPORTED

#include <string>
#include <memory>

namespace sdk {
	namespace network {

#if OPENSSL_SUPPORTED

		using SSL_unique_ptr = std::unique_ptr<SSL, decltype(&SSL_free)>;
		using X509_unique_ptr = std::unique_ptr<X509, decltype(&X509_free)>;

		class SSLSocket; // forward declaration
		/*
		 *	Creates an instance of secure socket layer object via socket id
		 *	to use independent connection operations.
		 */
		class SOCKET_API SSLSocketDescriptor : public SocketDescriptor {
		public:
			explicit SSLSocketDescriptor(SOCKET socketId, const SSLSocket& sSocket);
			~SSLSocketDescriptor() override;
			/*
			 *	This method used for validating hostname for certificate verification.
			 *	param1: Hostname such as www.sdk.com.
			 *	returns: nothing.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			void setHostname(const char* hostname);
			/*
			 *	Duty of connect method is connected to server for client applications using SSL.
			 *	returns: nothing
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			void connect();

			/*
			 *	This method used for reading operations from related secure socket layer.
			 *	param1: One byte character to read.
			 *	returns: Return byte count that read.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD std::size_t read(char& msgByte) const override;
			/*
			 *	This method used for reading operations from related secure socket layer.
			 *	param1: Bytes of vector.
			 *	param2: max read size.
			 *	returns: Return byte count that read.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD std::size_t read(std::vector<unsigned char>& message, int maxSize = 0) const override;
			/*
			 *	This method used for reading operations from related secure socket layer.
			 *	param1: String.
			 *	param2: max read size.
			 *	returns: Return byte count that read.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD std::size_t read(std::string& message, int maxSize = 0) const override;

			/*
			 *	This method used for writing operations from related secure socket layer.
			 *	param1: initializer_list of data via modern c++.
			 *	returns: Return byte count that write.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD int write(std::initializer_list<char> dataList) const override;

			/*
			 *	This method used for writing operations from related secure socket layer.
			 *	param1: Bytes of message.
			 *	param2: Size of message.
			 *	returns: Return byte count that write.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD int write(const char* data, int dataSize) const override;
			/*
			 *	This method used for writing operations from related secure socket layer.
			 *	param1: Bytes of vector message.
			 *	param2: Size of message.
			 *	returns: Return byte count that write.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD int write(const std::vector<unsigned char>& message) const override;
			/*
			 *	This method used for writing operations from related secure socket layer.
			 *	param1: String of message.
			 *	param2: Size of message.
			 *	returns: Return byte count that write.
			 *	exception: This method throws an SSLSocketException if an error occurs.
			 */
			NODISCARD int write(const std::string& message) const override;
			/*
			 *	Gets a socket id from related socket.
			 *	returns: The id of socket.
			 *	exception: This function never throws an exception.
			 */
			void accept();

		protected:
			std::string read(int maxSize = 0) const override;

		private:
			std::string m_hostname;
			SSL_unique_ptr m_ssl;
		};
#endif // OPENSSL_SUPPORTED
	}
}

#endif	// SSL_SOCKET_DESCRIPTOR_H
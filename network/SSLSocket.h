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

#ifndef SSL_SOCKET_H
#define SSL_SOCKET_H

#include "Socket.h"
#include "SSLSocketDescriptor.h"

#if OPENSSL_SUPPORTED
#include <openssl/crypto.h>
#endif // OPENSSL_SUPPORTED

//#include <functional>

namespace sdk {
	namespace network {

		/*
		 *	This class used for create a secure socket layer on socket.
		 *	It has some methods that using fill SSL attributes.
		 */

#if OPENSSL_SUPPORTED

		using CertVerifyCallback = std::function<int(int,X509_STORE_CTX*)>;
		using SSLCtx_unique_ptr = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;

		class SOCKET_API SSLSocket : public Socket {
		public:
			explicit SSLSocket(int port, ConnMethod meth,
				ProtocolType type = ProtocolType::tcp, IpVersion IpVer = IpVersion::IPv4);
			~SSLSocket() override = default;

			// non copyable
			SSLSocket(const SSLSocket&) = delete;
			SSLSocket& operator=(const SSLSocket&) = delete;

			/*
			 *	This function creates an instance of secure socket descriptor.
			 *	param1: The id of socket.
			 *	returns: A shared pointer of secure socket descriptor.
			 */
			NODISCARD std::shared_ptr<SSLSocketDescriptor> createSocketDescriptor(SOCKET socketId) const;
			/*
			 *	This function sets the list of available ciphers for ctx using the control string str.
			 *	If this function not use, the default cipher list is "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256".
			 *	param1:	Use this url https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_cipher_list.html,
			 *	and you can see usable list of cipher list.
			 *	returns: nothing.
			 *	exception: This function throws an SSLSocketException if an error occurs.
			 */
			void setCipherList(const char* str) const;
			/*
			 *	This function load the certificates in the given file path.
			 *	param1: File path.
			 *	returns: nothing.
			 *	exception: This function throws an SSLSocketException if an error occurs.
			 */
			void loadCertificateFile(const char* certFile, int type = SSL_FILETYPE_PEM) const;
			/*
			 *	This function load the key in the given file path.
			 *	param1: File path.
			 *	returns: nothing.
			 *	exception: This function throws an SSLSocketException if an error occurs.
			 */
			void loadPrivateKeyFile(const char* keyFile, int type = SSL_FILETYPE_PEM) const;
			
			/*
			 *	This function specifies the locations for ctx, at which CA certificates for verification purposes are located.
			 *	The certificates available via CAfile and CApath are trusted.
			 *	param1: Certificate file path.
			 *	param2: nullptr.
			 *	returns: nothing.
			 *	exception: This function throws an SSLSocketException if an error occurs.
			 */
			void loadVerifyLocations(const char* caFile, const char* caPath) const;
			void loadClientCertificateList(const char* path) const;
			void setVerifyDepth(int depth) const noexcept;
			/*
			 *	Gets a context object.
			 *	returns: The pointer address of SSL_CTX object created, otherwise nullptr.
			 *	exception : This function never throws an exception.
			 */
			NODISCARD SSL_CTX* getSSLCtx() const noexcept
			{
				return m_ctx.get();
			}

			/*
			 *	returns version number of OpenSSL library we use.
			 *	returns: Version number of OpenSSL library.
			 *	exception: This function never throws an exception.
			 */

			static const char* get_openssl_version() noexcept
			{
#if (OPENSSL_VERSION_NUMBER < 0x30000000L)
				return OpenSSL_version(OPENSSL_VERSION);
#else
				return OpenSSL_version(OPENSSL_FULL_VERSION_STRING);
#endif
			}
			
			void setVerifyCallback(const CertVerifyCallback& callback);

		private:
			static int verifyCallbackFunc(int preverifyOK, X509_STORE_CTX* x509Ctx);
			SSLCtx_unique_ptr m_ctx;
			CertVerifyCallback m_verifyCallback;
		};
#endif // OPENSSL_SUPPORTED
	}
}

#endif	// SSL_SOCKET_H
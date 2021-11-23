#ifndef SECURE_SOCKET_H_
#define SECURE_SOCKET_H_

#include "Socket.h"
#include "SecureSocketObject.h"

#if OPENSSL_SUPPORTED
#include <openssl/crypto.h>
#endif // OPENSSL_SUPPORTED

//#include <functional>

namespace sdk {
	namespace network {

		//using verify_cert_callback_t = std::function<int(int, X509_STORE_CTX* x509_ctx)>;
		/*
		*	This class used for create a secure socket layer on socket.
		*	It has some methods that using fill SSL attributes.
		*	Before using this class methods, do not forget calling SSLLibraryInit once if you are using
		*	OpenSSL 1.0.2 or below. Otherwise all methods fails.
		*/

#if OPENSSL_SUPPORTED

		class SecureSocket : public Socket {
		public:
			SecureSocket(int port, connection_method meth, protocol_type type = protocol_type::tcp , IpVersion IpVer = IpVersion::IPv4);
			~SecureSocket() override;

			//non copyable
			SecureSocket(const SecureSocket&) = delete;
			SecureSocket& operator=(const SecureSocket&) = delete;

			static bool SSLLibraryInit();
			/*
			*	This function creates an instance of secure socket layer object.
			*	param1: The id of socket.
			*	returns: A shared pointer of secure socket layer object.
			*/
			std::shared_ptr<SecureSocketObj> createnewSocket(SOCKET socket_id) const;
			/*
			*	This function sets the list of available ciphers for ctx using the control string str.
			*	If this function not use, the default ciphelist is "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256".
			*	param1:	Use this url https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_cipher_list.html
			*	and you can see useable list of chipher list.
			*	returns: nothing.
			*	exception: This function throws an SecureSocketException if an error occurs.
			*/
			void setCipherList(const char* str) const;
			/*
			*	This function load the certificates in the given file path.
			*	param1: File path.
			*	returns: nothing.
			*	exception: This function throws an SecureSocketException if an error occurs.
			*/
			void loadCertificateFile(const char* cert_file, int type = SSL_FILETYPE_PEM) const;
			/*
			*	This function load the key in the given file path.
			*	param1: File path.
			*	returns: nothing.
			*	exception: This function throws an SecureSocketException if an error occurs.
			*/
			void loadPrivateKeyFile(const char* key_file, int type = SSL_FILETYPE_PEM) const;
			/*
			*	This function sets the verification flags for ctx to be mode and specifies the verify_callback function to be used.
			*	param1: Exactly one of the mode flags SSL_VERIFY_NONE and SSL_VERIFY_PEER must be set at any time.
			*	param2: If no callback function shall be specified, the nullptr can be used for verify_callback.
			*	returns: nothing.
			*	exception: This function throws an SecureSocketException if an error occurs.
			*/
			void setCallbackVerifyCertificate(int mode, SSL_verify_cb callback) const noexcept;
			/*
			*	This function specifies the locations for ctx, at which CA certificates for verification purposes are located.
			*	The certificates available via CAfile and CApath are trusted.
			*	param1: Certificate file path.
			*	param2: nullptr.
			*	returns: nothing.
			*	exception: This function throws an SecureSocketException if an error occurs.
			*/
			void loadVerifyLocations(const char* ca_file, const char* ca_path) const;
			void loadClientCertificateList(const char* path) const;
			void setVerifyDepth(int depth) const noexcept;
			/*
			*	Gets a context object.
			*	returns: Error message.
			*	exception : This function never throws an exception.
			*/
			SSL_CTX* get_ctx() const noexcept {
				return m_ctx;
			}

			/*
			*	returns version number of OpenSSL library we use.
			*	returns: Version number of OpenSSL library.
			*	exception: This function never throws an exception.
			*/

			static const char * get_openssl_version() noexcept {
				return OpenSSL_version(OPENSSL_FULL_VERSION_STRING);
			}

		private:
			static bool m_bSSLLibraryInit;
		protected:
			SSL_CTX* m_ctx{};
		};
#endif // OPENSSL_SUPPORTED
	};
};

#endif
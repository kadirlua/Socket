#include "SecureSocket.h"
#include "general/SocketException.h"

namespace sdk{
	namespace network {

#if OPENSSL_SUPPORTED

		bool SecureSocket::m_bSSLLibraryInit = false;

		SecureSocket::SecureSocket(int port, connection_method meth, protocol_type type, IpVersion IpVer) :
			Socket{ port ,type, IpVer }
		{
			switch (meth) {
			case connection_method::client:
				m_ctx = SSL_CTX_new(SSLv23_client_method());
				break;
			case connection_method::server:
				m_ctx = SSL_CTX_new(SSLv23_server_method());
				break;
			default:
				m_ctx = SSL_CTX_new(SSLv23_client_method());
				break;
			}

			// We want to support all versions of TLS >= 1.0, but not the deprecated
			// and insecure SSLv2 and SSLv3.  Despite the name, SSLv23_*_method()
			// enables support for all versions of SSL and TLS, and we then disable
			// support for the old protocols immediately after creating the context.
			SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
		}

		SecureSocket::~SecureSocket()
		{
			if (m_ctx != nullptr) {
				SSL_CTX_free(m_ctx);
			}
		}

		bool SecureSocket::SSLLibraryInit()
		{
			if (!m_bSSLLibraryInit) {
				/*	OpenSSL 1.0.2 or below, then you would use SSL_library_init. If you are
				*	using OpenSSL 1.1.0 or above, then the library will initialize
				*	itself automatically.
				*	https://wiki.openssl.org/index.php/Library_Initialization
				*/
#if OPENSSL_VERSION_NUMBER < 0x10100000L
				SSL_library_init();
				SSL_load_error_strings();
#endif
				m_bSSLLibraryInit = true;
			}
			return true;
		}

		void SecureSocket::setCipherList(const char* str) const{
			int ret_code{};
			if ((ret_code = SSL_CTX_set_cipher_list(m_ctx, str)) <= 0) {
				throw general::SecureSocketException(ret_code, "Error setting the cipher list.");
			}
		}

		void SecureSocket::loadCertificateFile(const char* cert_file, int type /*= SSL_FILETYPE_PEM*/) const
		{
			int ret_code{};
			if ((ret_code = SSL_CTX_use_certificate_file(m_ctx, cert_file, type)) <= 0) {
				throw general::SecureSocketException(ret_code, "Error setting the certificate file");
			}
		}

		void SecureSocket::loadPrivateKeyFile(const char* key_file, int type /*= SSL_FILETYPE_PEM*/) const
		{
			int ret_code{};
			if ((ret_code = SSL_CTX_use_PrivateKey_file(m_ctx, key_file, type)) <= 0) {
				throw general::SecureSocketException(ret_code, "Error setting the key file.");
			}

			if ((ret_code = SSL_CTX_check_private_key(m_ctx)) == 0) {
				throw general::SecureSocketException(ret_code, "Private key does not match the certificate public key.");
			}
		}

		void SecureSocket::setCallbackVerifyCertificate(int mode, SSL_verify_cb callback) const noexcept {
			/*Used only if client authentication will be used*/
			SSL_CTX_set_verify(m_ctx, mode, callback);
		}

		void SecureSocket::loadVerifyLocations(const char* ca_file, const char* ca_path) const {
			int ret_code = 0;
			if ((ret_code = SSL_CTX_load_verify_locations(m_ctx, ca_file, ca_path)) < 1) {
				throw general::SecureSocketException(ret_code, "Error setting the verify locations.");
			}
		}

		void SecureSocket::loadClientCertificateList(const char * path) const
		{
			auto stack_ptr = SSL_load_client_CA_file(path);
			if (!stack_ptr)
			{
				throw general::SecureSocketException(-1, "can not load client CA file");
			}

			SSL_CTX_set_client_CA_list(m_ctx, stack_ptr);
		}

		void SecureSocket::setVerifyDepth(int depth) const noexcept
		{
			SSL_CTX_set_verify_depth(m_ctx, depth);
		}

		std::shared_ptr<SecureSocketObj> SecureSocket::createnewSocket(SOCKET socket_id) const
		{
			return std::make_shared<SecureSocketObj>(socket_id, *this);
		}
#endif // OPENSSL_SUPPORTED
	}
}
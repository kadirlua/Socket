// MIT License

// Copyright (c) 2021-2026 kadirlua

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

#include "SSLSocket.h"
#include "SocketException.h"

namespace sdk {
	namespace network {

#if OPENSSL_SUPPORTED

		int SSLSocket::verifyCallbackFunc(int preverifyOK, X509_STORE_CTX* x509Ctx)
		{
			/*
			 * Retrieve the pointer to the SSL of the connection currently treated
			 * and the application specific data stored into the SSL object.
			 */
			const auto* ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(x509Ctx,
				SSL_get_ex_data_X509_STORE_CTX_idx()));
			if (ssl != nullptr) {
				const auto* sslCtx = SSL_get_SSL_CTX(ssl);
				if (sslCtx != nullptr) {
					auto* sslServ = static_cast<SSLSocket*>(SSL_CTX_get_ex_data(sslCtx, 0));
					if (sslServ != nullptr && sslServ->m_verifyCallback) {
						return sslServ->m_verifyCallback(preverifyOK, x509Ctx);
					}
				}
			}
			return preverifyOK;
		}

		SSLSocket::SSLSocket(int port, ConnMethod meth, ProtocolType type, IpVersion IpVer) :
			Socket{ port, type, IpVer },
			m_ctx{ SSL_CTX_new(TLS_client_method()), SSL_CTX_free }
		{
			if (meth == ConnMethod::server) {
				m_ctx = SSLCtx_unique_ptr{ SSL_CTX_new(TLS_server_method()), SSL_CTX_free };
				SSL_CTX_set_ex_data(m_ctx.get(), 0, this);
				SSL_CTX_set_verify(m_ctx.get(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
					&SSLSocket::verifyCallbackFunc);
			}

			// We want to support all versions of TLS >= 1.0, but not the deprecated
			// and insecure SSLv2 and SSLv3.  Despite the name, SSLv23_*_method()
			// enables support for all versions of SSL and TLS, and we then disable
			// support for the old protocols immediately after creating the context.
			SSL_CTX_set_options(m_ctx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
		}

		void SSLSocket::setCipherList(const char* str)
		{
			const int retCode = SSL_CTX_set_cipher_list(m_ctx.get(), str);
			if (retCode <= 0) {
				throw general::SSLSocketException(retCode, "Error setting the cipher list.");
			}
		}

		void SSLSocket::loadCertificateFile(const char* certFile, int type /*= SSL_FILETYPE_PEM*/)
		{
			const int retCode = SSL_CTX_use_certificate_file(m_ctx.get(), certFile, type);
			if (retCode <= 0) {
				throw general::SSLSocketException(retCode, "Error setting the certificate file");
			}
		}

		void SSLSocket::loadPrivateKeyFile(const char* keyFile, int type /*= SSL_FILETYPE_PEM*/)
		{
			int retCode = SSL_CTX_use_PrivateKey_file(m_ctx.get(), keyFile, type);
			if (retCode <= 0) {
				throw general::SSLSocketException(retCode, "Error setting the key file.");
			}

			retCode = SSL_CTX_check_private_key(m_ctx.get());
			if (retCode == 0) {
				throw general::SSLSocketException(retCode, "Private key does not match the certificate public key.");
			}
		}

		void SSLSocket::loadVerifyLocations(const char* caFile, const char* caPath)
		{
			const int retCode = SSL_CTX_load_verify_locations(m_ctx.get(), caFile, caPath);
			if (retCode < 1) {
				throw general::SSLSocketException(retCode, "Error setting the verify locations.");
			}
		}

		void SSLSocket::loadClientCertificateList(const char* path)
		{
			auto* stackPtr = SSL_load_client_CA_file(path);
			if (stackPtr == nullptr) {
				throw general::SSLSocketException(-1, "can not load client CA file");
			}

			SSL_CTX_set_client_CA_list(m_ctx.get(), stackPtr);
		}

		void SSLSocket::setVerifyDepth(int depth) noexcept
		{
			SSL_CTX_set_verify_depth(m_ctx.get(), depth);
		}

		std::shared_ptr<SSLSocketDescriptor> SSLSocket::createSocketDescriptor(SOCKET socketId)
		{
			return std::make_shared<SSLSocketDescriptor>(socketId, *this);
		}

		void SSLSocket::setVerifyCallback(const CertVerifyCallback& callback)
		{
			m_verifyCallback = callback;
		}
#endif // OPENSSL_SUPPORTED
	}
}
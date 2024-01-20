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

#include "SSLServer.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"

#include <iostream>

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		namespace {
			constexpr const auto MAX_CLIENTS = 10;
			// only for testing for now
			const char* const certFile = R"(C:\Program Files\OpenSSL\bin\certificate.pem)";
			const char* const keyFile = R"(C:\Program Files\OpenSSL\bin\key.key)";

			int verifyCallback(int preverify_ok, X509_STORE_CTX* x509Ctx)
			{
				const auto* cert = X509_STORE_CTX_get_current_cert(x509Ctx);
				if (cert == nullptr) {
					return 0;
				}

				int const errCode = X509_STORE_CTX_get_error(x509Ctx);
				int check = 0;
				if (errCode != X509_V_OK) {
					return errCode;
				}

				const auto* subjectName = X509_get_subject_name(cert);
				if (subjectName == nullptr) {
					return 0;
				}

				char buf[6][256]{};
				std::cout << "Certificate subject:\n";
				check = X509_NAME_get_text_by_NID(subjectName, NID_commonName, buf[0], sizeof(buf[0]));
				if (check > 0) {
					std::cout << " - Common name: " << buf[0] << "\n";
				}
				check = X509_NAME_get_text_by_NID(subjectName, NID_organizationName, buf[1], sizeof(buf[1]));
				if (check > 0) {
					std::cout << " - Organization name: " << buf[1] << "\n";
				}
				check = X509_NAME_get_text_by_NID(subjectName, NID_organizationalUnitName, buf[2], sizeof(buf[2]));
				if (check > 0) {
					std::cout << " - Organizational unit name: " << buf[2] << "\n";
				}
				std::cout << "Certificate issuer:\n";
				check = X509_NAME_get_text_by_NID(subjectName, NID_commonName, buf[3], sizeof(buf[3]));
				if (check > 0) {
					std::cout << " - Common name: " << buf[3] << "\n";
				}
				check = X509_NAME_get_text_by_NID(subjectName, NID_organizationName, buf[4], sizeof(buf[4]));
				if (check > 0) {
					std::cout << " - Organization name: " << buf[4] << "\n";
				}
				check = X509_NAME_get_text_by_NID(subjectName, NID_organizationalUnitName, buf[5], sizeof(buf[5]));
				if (check > 0) {
					std::cout << " - Organizational unit name: " << buf[5] << "\n";
				}

				return preverify_ok;
			}
		}

		SSLServer::SSLServer(int port, network::ProtocolType type, network::IpVersion ipVer) :
			m_socket{ port, network::ConnMethod::server, type, ipVer }
		{
		}

		void SSLServer::startListening()
		{
			const network::SocketOption<network::SSLSocket> socketOpt{ m_socket };
			socketOpt.setBlockingMode(1); // non-blocking mode
			socketOpt.setReuseAddr(1);

			// set certificate properties
			m_socket.loadVerifyLocations(certFile, nullptr);
			/* Load the client's CA file location as well */
			// m_socket_ptr->loadClientCertificateList("client.pem");
			// m_socket_ptr->setCipherList("AES128-SHA");
			m_socket.loadCertificateFile(certFile);
			m_socket.loadPrivateKeyFile(keyFile);

			m_socket.setCallbackVerifyCertificate(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyCallback);
			/* We accept only certificates signed only by the CA himself */
			// m_socket.setVerifyDepth(1);
			// bind and listen
			m_socket.bind();
			m_socket.listen(MAX_CLIENTS);

			const std::string response{ "Hello from SSLServer!\n" };

			while (true) {
				try {
					const SOCKET newSockId = m_socket.accept();
					auto sslSocketDesc = m_socket.createSocketDescriptor(newSockId);
					sslSocketDesc->accept();
					std::string requestMsg;
					sslSocketDesc->read(requestMsg);
					std::cout << "Message recieved from client: " << requestMsg << "\n";
					sslSocketDesc->write(response);
				}
				catch (const general::SocketException& ex) {
					(void)ex;
				}
			}
		}

#endif // OPENSSL_SUPPORTED
	}
}

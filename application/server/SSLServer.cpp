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
		}

		SSLServer::SSLServer(int port, network::ProtocolType type, network::IpVersion ipVer) :
			Server{ port, type, ipVer },
			m_sslSocket{ port, network::ConnMethod::server, type, ipVer }
		{
		}

		void SSLServer::startListening()
		{
			const network::SocketOption<network::SSLSocket> socketOpt{ m_sslSocket };
			socketOpt.setBlockingMode(network::SocketOpt::ON); // non-blocking mode
			socketOpt.setReuseAddr(network::SocketOpt::ON);

			// bind and listen
			m_sslSocket.bind();
			m_sslSocket.listen(MAX_CLIENTS);

			const std::string response{ "Hello from SSLServer!\n" };

			while (!isAbortedListening()) {
				try {
					const SOCKET newSockId = m_sslSocket.accept();
					auto sslSocketDesc = m_sslSocket.createSocketDescriptor(newSockId);
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

		void SSLServer::loadServerCertificate(const char* certFile) const
		{
			m_sslSocket.loadCertificateFile(certFile);
		}

		void SSLServer::loadServerPrivateKey(const char* keyFile) const
		{
			m_sslSocket.loadPrivateKeyFile(keyFile);
		}

		void SSLServer::loadServerVerifyLocations(const char* caFile, const char* caPath) const
		{
			m_sslSocket.loadVerifyLocations(caFile, caPath);
		}

		void SSLServer::setVerifyCallback(const network::CertVerifyCallback& callback)
		{
			m_sslSocket.setVerifyCallback(callback);
		}

#endif // OPENSSL_SUPPORTED
	}
}

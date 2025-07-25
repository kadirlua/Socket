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

#include "SSLClient.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		SSLClient::SSLClient(const std::string& ipAddr, int port,
			network::ProtocolType type /*= network::ProtocolType::tcp*/,
			network::IpVersion ipVer /*= network::IpVersion::IPv4*/) :
			Client{ ipAddr, port, type, ipVer },
			m_sslSocket{ port, network::ConnMethod::client, type, ipVer }
		{
			m_sslSocket.setIpAddress(ipAddr);
			m_sslSocket.setInterruptCallback([this](const network::Socket& socket) {
				(void)socket;
				return isConnectionAborted();
			});
		}

		void SSLClient::setCertificateAtr(const char* certFile, const char* keyFile)
		{
			m_sslSocket.loadCertificateFile(certFile);
			m_sslSocket.loadPrivateKeyFile(keyFile);
		}

		void SSLClient::connectServer()
		{
			network::SocketOption<network::SSLSocket> socketOpt{ m_sslSocket };
			socketOpt.setBlockingMode(network::SocketOpt::ON); // non-blocking mode
			m_sslSocket.connect();
			m_sslSocketDesc = m_sslSocket.createSocketDescriptor(m_sslSocket.getSocketId());
			m_sslSocketDesc->connect();
		}

		int SSLClient::write(std::initializer_list<char> msg) const
		{
			return m_sslSocketDesc->write(msg);
		}

		int SSLClient::write(const char* msg, int msgSize) const
		{
			return m_sslSocketDesc->write(msg, msgSize);
		}

		int SSLClient::write(const std::string& msg) const
		{
			return m_sslSocketDesc->write(msg);
		}

		int SSLClient::write(const std::vector<unsigned char>& msg) const
		{
			return m_sslSocketDesc->write(msg);
		}

		std::size_t SSLClient::read(std::vector<unsigned char>& responseMsg, int maxSize /*= 0*/) const
		{
			return m_sslSocketDesc->read(responseMsg, maxSize);
		}

		std::size_t SSLClient::read(std::string& message, int maxSize /*= 0*/) const
		{
			return m_sslSocketDesc->read(message, maxSize);
		}

#endif // OPENSSL_SUPPORTED
	}
}
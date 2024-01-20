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

#include "SSLClient.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		SSLClient::SSLClient(const std::string& ipAddr, int port, 
							network::ProtocolType type /*= network::ProtocolType::tcp*/, 
							network::IpVersion ipVer /*= network::IpVersion::IPv4*/) :
			m_socket{ port, network::ConnMethod::client, type, ipVer }
		{
			m_socket.setIpAddress(ipAddr);
			m_socket.setInterruptCallback([this](const network::Socket& socket) {
				(void)socket;
				return isConnectionAborted();
			});
		}

		void SSLClient::setCertificateAtr(const char* certFile, const char* keyFile) const
		{
			m_socket.loadCertificateFile(certFile);
			m_socket.loadPrivateKeyFile(keyFile);
		}

		void SSLClient::connectServer()
		{
			const network::SocketOption<network::SSLSocket> socketOpt{ m_socket };
			socketOpt.setBlockingMode(1); // non-blocking mode
			m_socket.connect();
			m_secureDesc = m_socket.createSocketDescriptor(m_socket.getSocketId());
			m_secureDesc->connect();
		}

		int SSLClient::write(std::initializer_list<char> msg) const
		{
			return m_secureDesc->write(msg);
		}

		int SSLClient::write(const char* msg, int msgSize) const
		{
			return m_secureDesc->write(msg, msgSize);
		}

		int SSLClient::write(const std::string& msg) const
		{
			return m_secureDesc->write(msg);
		}

		int SSLClient::write(const std::vector<unsigned char>& msg) const
		{
			return m_secureDesc->write(msg);
		}

		std::size_t SSLClient::read(std::vector<unsigned char>& responseMsg, int maxSize /*= 0*/) const
		{
			return m_secureDesc->read(responseMsg, maxSize);
		}

		std::size_t SSLClient::read(std::string& message, int maxSize /*= 0*/) const
		{
			return m_secureDesc->read(message, maxSize);
		}

		void SSLClient::abortConnection() noexcept
		{
			m_abortConnection = true;
		}
#endif // OPENSSL_SUPPORTED
	}
}
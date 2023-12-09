// MIT License

// Copyright (c) 2021-2023 kadirlua

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

#include "SecureClient.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

		using namespace network;

#if OPENSSL_SUPPORTED

		namespace {
			bool callbackInterrupt(void* userdata_ptr)
			{
				auto* client = static_cast<SecureClient*>(userdata_ptr);
				return client->isInterrupted();
			}
		}

		SecureClient::SecureClient(const std::string& ip, int port, network::protocol_type type /*= network::protocol_type::tcp*/, network::IpVersion ipVer /*= network::IpVersion::IPv4*/) :
			m_socket_ptr{ std::make_unique<SSLSocket>(port, connection_method::client, type, ipVer) }

		{
			m_socket_ptr->setIpAddress(ip);
			const SocketOption<SSLSocket> socketOpt{ *m_socket_ptr };
			socketOpt.setBlockingMode(1); // non-blocking mode
			m_socket_ptr->setInterruptCallback(&callbackInterrupt, this);
			m_socket_ptr->connect();
		}

		void SecureClient::setCertificateAtr(const char* cert_file, const char* key_file) const
		{
			m_socket_ptr->loadCertificateFile(cert_file);
			m_socket_ptr->loadPrivateKeyFile(key_file);
		}

		void SecureClient::connectServer()
		{
			m_secure_obj = m_socket_ptr->createNewSocket(m_socket_ptr->getSocketId());
			m_secure_obj->connect();
		}

		int SecureClient::write(std::initializer_list<char> msg) const
		{
			return m_secure_obj->write(msg);
		}

		int SecureClient::write(const char* msg, int msg_size) const
		{
			return m_secure_obj->write(msg, msg_size);
		}

		int SecureClient::write(const std::string& msg) const
		{
			return m_secure_obj->write(msg);
		}

		int SecureClient::write(const std::vector<unsigned char>& msg) const
		{
			return m_secure_obj->write(msg);
		}

		std::size_t SecureClient::read(std::vector<unsigned char>& response_msg, int max_size /*= 0*/) const
		{
			return m_secure_obj->read(response_msg, max_size);
		}

		std::size_t SecureClient::read(std::string& message, int max_size /*= 0*/) const
		{
			return m_secure_obj->read(message);
		}
#endif // OPENSSL_SUPPORTED
	}
}
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

#pragma once
#include "Client.h"
#include "network/SSLSocket.h"
#include <string>

namespace sdk {
	namespace application {

#if OPENSSL_SUPPORTED

		class SOCKET_API SSLClient : public Client {
		public:
			SSLClient(const std::string& ipAddr, int port,
				network::ProtocolType type = network::ProtocolType::tcp,
				network::IpVersion ipVer = network::IpVersion::IPv4);
			~SSLClient() override = default;

			// non copyable
			SSLClient(const SSLClient&) = delete;
			SSLClient& operator=(const SSLClient&) = delete;

			void setCertificateAtr(const char* certFile, const char* keyFile);
			void connectServer() override;
			NODISCARD int write(std::initializer_list<char> msg) const override;
			NODISCARD int write(const char* msg, int msgSize) const override;
			NODISCARD int write(const std::string& msg) const override;
			NODISCARD int write(const std::vector<unsigned char>& msg) const override;
			NODISCARD std::size_t read(std::vector<unsigned char>& responseMsg, int maxSize = 0) const override;
			NODISCARD std::size_t read(std::string& message, int maxSize = 0) const override;

		private:
			network::SSLSocket m_sslSocket;
			std::shared_ptr<network::SSLSocketDescriptor> m_sslSocketDesc;
		};
#endif // OPENSSL_SUPPORTED
	}
}

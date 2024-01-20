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

#include "Server.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"

#include <iostream>

namespace sdk {
	namespace application {

		namespace {
			constexpr const auto MAX_CLIENTS = 10;
		}

		Server::Server(int port, 
						network::ProtocolType type /*= ProtocolType::tcp*/, 
						network::IpVersion ipVer /*= IpVersion::IPv4*/) :
			m_socket{ port, type, ipVer }
		{
		}

		void Server::startListening()
		{
			const network::SocketOption<network::Socket> socketOpt{ m_socket };
			socketOpt.setBlockingMode(1); // non-blocking mode
			socketOpt.setReuseAddr(1);

			// bind and listen
			m_socket.bind();
			m_socket.listen(MAX_CLIENTS);

			const std::string response{ "Hello from Server!\n" };

			while (!m_abortListening) {
				try {
					const SOCKET newSockId = m_socket.accept();
					auto socketDesc = m_socket.createSocketDescriptor(newSockId);
					std::string requestMsg;
					socketDesc->read(requestMsg);
					std::cout << "Message recieved from client: " << requestMsg << "\n";
					socketDesc->write(response);
				}
				catch (const general::SocketException& ex) {
					(void)ex;
				}
			}
		}

		void Server::abortListening() noexcept
		{
			m_abortListening = true;
		}
	}
}

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

#include "Client.h"
#include "network/SocketOption.h"

namespace sdk {
	namespace application {

		Client::Client(const std::string& ipAddr, int port,
			network::ProtocolType type /*= ProtocolType::tcp*/,
			network::IpVersion ipVer /*= IpVersion::IPv4*/) :
			m_socket{ port, type, ipVer }
		{
			m_socket.setIpAddress(ipAddr);
			m_socket.setInterruptCallback([this](const network::Socket& socket) {
				(void)socket;
				return isConnectionAborted();
			});
		}

		void Client::connectServer()
		{
			m_socket.connect();
			network::SocketOption<network::Socket> socketOpt{ m_socket };
			socketOpt.setBlockingMode(network::SocketOpt::ON);
			m_socketDesc = m_socket.createSocketDescriptor(m_socket.getSocketId());
		}

		int Client::write(std::initializer_list<char> msg) const
		{
			return m_socketDesc->write(msg);
		}

		int Client::write(const char* msg, int msgSize) const
		{
			return m_socketDesc->write(msg, msgSize);
		}

		int Client::write(const std::string& msg) const
		{
			return m_socketDesc->write(msg);
		}

		int Client::write(const std::vector<unsigned char>& msg) const
		{
			return m_socketDesc->write(msg);
		}

		std::size_t Client::read(std::vector<unsigned char>& responseMsg, int maxSize /*= 0*/) const
		{
			return m_socketDesc->read(responseMsg, maxSize);
		}

		std::size_t Client::read(std::string& message, int maxSize /*= 0*/) const
		{
			return m_socketDesc->read(message, maxSize);
		}

		void Client::abortConnection() noexcept
		{
			m_abortConnection = true;
		}
	}
}
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

#include <iostream>
#include <cstdlib>
#include <limits>

#include "application/client/SSLClient.h"
#include "general/SocketException.h"

namespace {
	constexpr const auto WSA_VERSION = 0x202;
}

int main(int argc, const char** argv)
{
	if (argc < 4) {
		std::cout << "Missing argument.\r\n Usage <exe_name> <ip_address> <port_number> <message>";
		return -1;
	}

	const std::string strIpAddress = argv[1];

	const auto portNumber = std::atoi(argv[2]);
	if (portNumber <= 0 || portNumber > std::numeric_limits<std::uint16_t>::max()) {
		std::cout << "Invalid port range.";
		return -1;
	}

	const std::string strMsg = argv[3];
	
	if (!sdk::network::Socket::WSAInit(WSA_VERSION)) {
		std::cout << "sdk::network::Socket::WSAInit failed\r\n";
		return -1;
	}

	try {
		sdk::application::SSLClient client{ strIpAddress, portNumber };
		client.connectServer();
		client.write(strMsg);
		std::string strResponse;
		client.read(strResponse);
		std::cout << "Response from server: " << strResponse << "\r\n";
	} catch (const sdk::general::SSLSocketException& ex) {
		std::cout << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
	}

	sdk::network::Socket::WSADeinit();
	return 0;
}

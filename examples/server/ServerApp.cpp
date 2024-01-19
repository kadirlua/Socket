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

// SocketExample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <limits>

#include "application/server/Server.h"
#include "general/SocketException.h"

namespace {
	constexpr const auto WSA_VERSION = 0x202;
}

int main(int argc, const char** argv)
{
	if (argc < 2) {
		std::cout << "Missing argument.\r\n Usage <exe_name> <port_number>";
		return -1;
	}

	const auto portNumber = std::atoi(argv[1]);
	if (portNumber <= 0 || portNumber > std::numeric_limits<std::uint16_t>::max()) {
		std::cout << "Invalid port range.";
		return -1;
	}
	
	if (!sdk::network::Socket::WSAInit(WSA_VERSION)) {
		std::cout << "sdk::network::Socket::WSAInit failed\r\n";
		return -1;
	}

	try {
		sdk::application::Server srv{ portNumber };
		srv.startListening();
	} catch (const sdk::general::SocketException& ex) {
		std::cout << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
	}

	sdk::network::Socket::WSADeinit();
	return 0;
}

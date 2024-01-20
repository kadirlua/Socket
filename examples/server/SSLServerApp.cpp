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

#include "application/server/SSLServer.h"
#include "general/SocketException.h"

namespace {
	// only for testing for now
	const char* const certFile = R"(C:\Program Files\OpenSSL\bin\certificate.pem)";
	const char* const keyFile = R"(C:\Program Files\OpenSSL\bin\key.key)";

	int verifyCallback(int preverifyOK, X509_STORE_CTX* x509Ctx)
	{
		const auto* cert = X509_STORE_CTX_get_current_cert(x509Ctx);
		if (cert == nullptr) {
			return 0;
		}

		const int errCode = X509_STORE_CTX_get_error(x509Ctx);
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

		return preverifyOK;
	}
}

int main(int argc, const char** argv)
{
#if OPENSSL_SUPPORTED
	if (argc < 2) {
		std::cout << "Missing argument.\r\nUsage <exe_name> <port_number>";
		return -1;
	}

	const auto portNumber = std::atoi(argv[1]);
	if (portNumber <= 0 || portNumber > std::numeric_limits<std::uint16_t>::max()) {
		std::cout << "Invalid port range.\r\n";
		return -1;
	}
	
	if (!sdk::network::Socket::WSAInit(sdk::network::WSA_VER_2_2)) {
		std::cout << "sdk::network::Socket::WSAInit failed\r\n";
		return -1;
	}

	try {
		sdk::application::SSLServer srv{ portNumber };
		srv.loadServerCertificate(certFile);
		srv.loadServerPrivateKey(keyFile);
		srv.loadServerVerifyLocations(certFile, nullptr);
		srv.setVerifyCallback(verifyCallback);
		srv.startListening();
	} catch (const sdk::general::SSLSocketException& ex) {
		std::cout << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\r\n";
	}

	sdk::network::Socket::WSADeinit();
#else
	std::cout << "Build the project with OPENSSL_SUPPORTED.\r\n";
#endif	// OPENSSL_SUPPORTED
	return 0;
}

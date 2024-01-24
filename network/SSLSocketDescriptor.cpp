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

#include "SSLSocketDescriptor.h"
#include "general/SocketException.h"
#include "SSLSocket.h"
#include <iterator>

#if OPENSSL_SUPPORTED
#include <openssl/x509v3.h>	// required for host verification
#endif // OPENSSL_SUPPORTED

namespace sdk {
	namespace network {

#if OPENSSL_SUPPORTED

		/**************************Secure Object Part**************************/
		SSLSocketDescriptor::SSLSocketDescriptor(SOCKET socketId, const SSLSocket& sSocket) :
			SocketDescriptor{ socketId, sSocket },
			m_ssl{ SSL_new(sSocket.getSSLCtx()), SSL_free }
		{
			if (m_ssl != nullptr) {
				if (SSL_set_fd(m_ssl.get(), static_cast<int>(socketId)) == 0) {
					throw general::SSLSocketException(SSL_get_error(m_ssl.get(), 0));
				}
			}
		}

		SSLSocketDescriptor::~SSLSocketDescriptor()
		{
			int ret = 0;
			bool bDone = false;
			while ((ret = SSL_shutdown(m_ssl.get())) <= 0 && !bDone) {
				switch (const int errCode = SSL_get_error(m_ssl.get(), ret)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					break;
				default:
					bDone = true;
					break;
				}
			}
		}

		void SSLSocketDescriptor::setHostname(const char* hostname)
		{
			if (SSL_set_tlsext_host_name(m_ssl.get(), hostname) != 1) {
				throw general::SSLSocketException("set host name failed!");
			}

			m_hostname = hostname;
		}

		void SSLSocketDescriptor::connect()
		{
			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			int retCode{};
			while ((retCode = SSL_connect(m_ssl.get())) == -1) {
				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (const int errCode = SSL_get_error(m_ssl.get(), retCode)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_CONNECT:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
					[[fallthrough]];
#endif
				default:
					throw general::SSLSocketException(errCode);
				}
			}
		}

		void SSLSocketDescriptor::accept() // this function used for handshake
		{
			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			int retCode{};
			while ((retCode = SSL_accept(m_ssl.get())) != 1) {
				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (const int errCode = SSL_get_error(m_ssl.get(), retCode)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_ACCEPT:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
					[[fallthrough]];
#endif
				default:
					throw general::SSLSocketException(errCode);
				}
			}

 			const X509_unique_ptr peer{ SSL_get_peer_certificate(m_ssl.get()), X509_free };
			if (peer) {
				const long retCode = SSL_get_verify_result(m_ssl.get());
				if (retCode != X509_V_OK) {
					throw general::SSLSocketException(retCode);
				}

				// check host
				// if our server has valid certificate such an google.com uncomment this if block
				if (!m_hostname.empty()) {
					const int checkResult = X509_check_host(peer.get(), m_hostname.c_str(), 
						m_hostname.size(), 0, nullptr);
					if (checkResult != 1) {
						throw general::SSLSocketException(checkResult);
					}
				}
			}
			else {
				throw general::SSLSocketException("client did not give a certificate");	
			}
		}

		std::size_t SSLSocketDescriptor::read(char& msgByte) const
		{
			const int numBytes = SSL_read(m_ssl.get(), &msgByte, 1);
			if (numBytes < 0) {
				throw general::SSLSocketException(numBytes);
			}
			return static_cast<std::size_t>(numBytes);
		}

		std::string SSLSocketDescriptor::read(int maxSize /*= 0*/) const
		{
			const int bufLen = (maxSize > 0 && maxSize < MAX_MESSAGE_SIZE) ? maxSize : MAX_MESSAGE_SIZE - 1;

			std::string strMessage;
			std::vector<char> dataVec(bufLen);
			
			int receiveByte = 0;

			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			do {
				while ((receiveByte = SSL_read(m_ssl.get(), dataVec.data(), bufLen)) == -1) {
					if (callbackInterrupt &&
						callbackInterrupt(m_socketRef)) {
						throw general::SSLSocketException(INTERRUPT_MSG);
					}

					switch (const auto errCode = SSL_get_error(m_ssl.get(), receiveByte)) {
					case SSL_ERROR_WANT_READ:
						break;
					case SSL_ERROR_ZERO_RETURN:
						SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
						[[fallthrough]];
#endif
					default:
						throw general::SSLSocketException(errCode);
					}
				}

				if (receiveByte > 0) {
					std::move(dataVec.begin(), dataVec.begin() + receiveByte,
						std::back_inserter(strMessage));
				}

				if (SSL_pending(m_ssl.get()) == 0) {
					break;
				}

				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				if (maxSize > 0 && strMessage.size() >= (std::size_t)maxSize) {
					break;
				}

			} while (receiveByte > 0);

			return strMessage;
		}

		std::size_t SSLSocketDescriptor::read(std::vector<unsigned char>& message, int maxSize /*= 0*/) const
		{
			const auto receivedStr = read(maxSize);
			std::move(receivedStr.begin(), receivedStr.end(), std::back_inserter(message));
			return message.size();
		}

		std::size_t SSLSocketDescriptor::read(std::string& message, int maxSize /*= 0*/) const
		{
			message = read(maxSize);
			return message.size();
		}

		int SSLSocketDescriptor::write(std::initializer_list<char> dataList) const
		{
			return write(dataList.begin(), static_cast<int>(dataList.size()));
		}

		int SSLSocketDescriptor::write(const char* data, int dataSize) const
		{
			const auto& callbackInterrupt = m_socketRef.m_callbackInterrupt;

			int sendBytes{};
			while ((sendBytes = SSL_write(m_ssl.get(), data, dataSize)) == -1) {
				if (callbackInterrupt &&
					callbackInterrupt(m_socketRef)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (const auto errCode = SSL_get_error(m_ssl.get(), sendBytes)) {
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
					[[fallthrough]];
#endif
				default:
					throw general::SSLSocketException(errCode);
				}
			}
			return sendBytes;
		}

		int SSLSocketDescriptor::write(const std::vector<unsigned char>& message) const
		{
			const std::string strBuf{ message.begin(), message.end() };
			return write(strBuf.c_str(), static_cast<int>(strBuf.size()));
		}

		int SSLSocketDescriptor::write(const std::string& message) const
		{
			return write(message.c_str(),
				static_cast<int>(message.size()));
		}
#endif // OPENSSL_SUPPORTED
	}
}
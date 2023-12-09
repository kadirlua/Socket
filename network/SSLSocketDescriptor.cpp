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
			m_ssl{ SSL_new(sSocket.get_ctx()), SSL_free }
		{
			if (m_ssl != nullptr) {
				if (SSL_set_fd(m_ssl.get(), (int)socketId) == 0) {
					throw general::SSLSocketException(SSL_get_error(m_ssl.get(), 0));
				}
			}
		}

		SSLSocketDescriptor::~SSLSocketDescriptor()
		{
			/*int err = SSL_get_error(m_ssl, -1);
			if (err != SSL_ERROR_SYSCALL && err != SSL_ERROR_SSL)
			{*/
			int err = 0;
			bool bDone = false;
			while ((err = SSL_shutdown(m_ssl.get())) <= 0 && !bDone) {
				switch (const int ret_code = SSL_get_error(m_ssl.get(), err)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					break;
				default:
					bDone = true;
					break;
				}
			}
			//}
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
			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			int err_code{};
			while ((err_code = SSL_connect(m_ssl.get())) == -1) {
				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (const int ret_code = SSL_get_error(m_ssl.get(), err_code)) {
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
					throw general::SSLSocketException(ret_code);
				}
			}
		}

		void SSLSocketDescriptor::accept() // this function used for handshake
		{
			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			int err_code{};
			while ((err_code = SSL_accept(m_ssl.get())) != 1) {
				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (const int ret_code = SSL_get_error(m_ssl.get(), err_code)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_ACCEPT:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
					[[fallthrough]];
#endif
				default:
					throw general::SSLSocketException(ret_code);
				}
			}

 			auto peer = X509_unique_ptr{ SSL_get_peer_certificate(m_ssl.get()), X509_free };
			if (peer) {
				const long ret_code = SSL_get_verify_result(m_ssl.get());
				if (ret_code != X509_V_OK) {
					throw general::SSLSocketException(ret_code);
				}

				// check host
				// if our server has valid certificate such an google.com uncomment this if block
				if (!m_hostname.empty()) {
					const int check_result = X509_check_host(peer.get(), m_hostname.c_str(), m_hostname.size(), 0, nullptr);
					if (check_result != 1) {
						throw general::SSLSocketException(check_result);
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

		std::string SSLSocketDescriptor::read(int max_size /*= 0*/) const
		{
			const int buf_len = (max_size > 0 && max_size < MAX_MESSAGE_SIZE) ? max_size : MAX_MESSAGE_SIZE - 1;

			std::string str_message;
			std::vector<char> dataVec;
			dataVec.resize(buf_len);

			int receive_byte = 0;

			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			do {
				while ((receive_byte = SSL_read(m_ssl.get(), dataVec.data(), buf_len)) == -1) {
					if (callback_interrupt &&
						callback_interrupt(m_socket_ref.m_userdata_ptr)) {
						throw general::SSLSocketException(INTERRUPT_MSG);
					}

					switch (auto err_code = SSL_get_error(m_ssl.get(), receive_byte)) {
					case SSL_ERROR_WANT_READ:
						break;
					case SSL_ERROR_ZERO_RETURN:
						SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
						[[fallthrough]];
#endif
					default:
						throw general::SSLSocketException(err_code);
					}
				}

				if (receive_byte > 0) {
					std::move(dataVec.begin(), dataVec.begin() + receive_byte,
						std::back_inserter(str_message));
				}

				if (SSL_pending(m_ssl.get()) == 0) {
					break;
				}

				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				if (max_size > 0 && str_message.size() >= (std::size_t)max_size) {
					break;
				}

			} while (receive_byte > 0);

			return str_message;
		}

		std::size_t SSLSocketDescriptor::read(std::vector<unsigned char>& message, int max_size /*= 0*/) const
		{
			const auto received_str = read(max_size);
			std::move(received_str.begin(), received_str.end(), std::back_inserter(message));
			return message.size();
		}

		std::size_t SSLSocketDescriptor::read(std::string& message, int max_size /*= 0*/) const
		{
			message = read(max_size);
			return message.size();
		}

		int SSLSocketDescriptor::write(std::initializer_list<char> data_list) const
		{
			return write(data_list.begin(), (int)data_list.size());
		}

		int SSLSocketDescriptor::write(const char* data, int data_size) const
		{
			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			int sendBytes{};
			while ((sendBytes = SSL_write(m_ssl.get(), data, data_size)) == -1) {
				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SSLSocketException(INTERRUPT_MSG);
				}

				switch (auto err_code = SSL_get_error(m_ssl.get(), sendBytes)) {
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl.get());
#if (__cplusplus >= 201703L)
					[[fallthrough]];
#endif
				default:
					throw general::SSLSocketException(err_code);
				}
			}
			return sendBytes;
		}

		int SSLSocketDescriptor::write(const std::vector<unsigned char>& message) const
		{
			const std::string strBuf(message.begin(), message.end());
			return write(strBuf.c_str(), (int)strBuf.size());
		}

		int SSLSocketDescriptor::write(const std::string& message) const
		{
			return write(message.c_str(),
				static_cast<int>(message.size()));
		}
#endif // OPENSSL_SUPPORTED
	}
}
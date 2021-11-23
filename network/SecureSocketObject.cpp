#include "SecureSocketObject.h"
#include "general/SocketException.h"
#include "SecureSocket.h"
#include <iterator>

#if OPENSSL_SUPPORTED
#include <openssl/x509v3.h>	// required for host verification
#endif // OPENSSL_SUPPORTED

namespace sdk {
	namespace network {

#if OPENSSL_SUPPORTED

		/**************************Secure Object Part**************************/
		SecureSocketObj::SecureSocketObj(SOCKET socketid, const SecureSocket& ss) :
			SocketObj{ socketid, ss }
		{
			m_ssl = SSL_new(ss.get_ctx());
			if (m_ssl)
			{
				if (SSL_set_fd(m_ssl, (int)socketid) == 0)
				{
					throw general::SecureSocketException(SSL_get_error(m_ssl, 0));
				}
			}
		}

		SecureSocketObj::~SecureSocketObj()
		{
			/*int err = SSL_get_error(m_ssl, -1);
			if (err != SSL_ERROR_SYSCALL && err != SSL_ERROR_SSL)
			{*/
			int err;
			bool bDone = false;
			while ((err = SSL_shutdown(m_ssl)) <= 0 && !bDone)
			{
				switch (int ret_code = SSL_get_error(m_ssl, err))
				{
				case SSL_ERROR_WANT_READ:
					break;
				case SSL_ERROR_WANT_WRITE:
					break;
				default:
					bDone = true;
					break;
				}
			}
			//}

			SSL_free(m_ssl);
		}

		void SecureSocketObj::setHostname(const char* hostname) {
			if (SSL_set_tlsext_host_name(m_ssl, hostname) != 1)
				throw general::SecureSocketException("set host name failed!");

			m_hostname = hostname;
		}

		void SecureSocketObj::connect()
		{
			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			int err_code{};
			while ((err_code = SSL_connect(m_ssl)) == -1)
			{
				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SecureSocketException(INTERRUPT_MSG);

				switch (int ret_code = SSL_get_error(m_ssl, err_code))
				{
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_CONNECT:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl);
#if _HAS_CXX17
					[[fallthrough]];
#endif
				default:
					throw general::SecureSocketException(ret_code);
				}
			}

		}

		void SecureSocketObj::accept()     //this function used for handshake
		{
			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			int err_code{};
			while ((err_code = SSL_accept(m_ssl)) != 1)
			{
				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SecureSocketException(INTERRUPT_MSG);

				switch (int ret_code = SSL_get_error(m_ssl, err_code))
				{
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_ACCEPT:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl);
#if _HAS_CXX17
					[[fallthrough]];
#endif
				default:
					throw general::SecureSocketException(ret_code);
				}
			}

			X509* peer = SSL_get_peer_certificate(m_ssl);
			if (peer)
			{
				long ret_code = SSL_get_verify_result(m_ssl);
				if (ret_code != X509_V_OK)
				{
					throw general::SecureSocketException(ret_code);
				}

				//check host 
				//if our server has valid certificate such an google.com uncomment this if block
				if (!m_hostname.empty()) {
					int check_result = X509_check_host(peer, m_hostname.c_str(), m_hostname.size(), 0, nullptr);
					if (check_result != 1)
					{
						throw general::SecureSocketException(check_result);
					}
				}
				
			}
			else
				throw general::SecureSocketException("client did not give a certificate");
		}

		size_t SecureSocketObj::read(char& msgByte) const
		{
			int numBytes = SSL_read(m_ssl, &msgByte, 1);
			if (numBytes < 0)
			{
				throw general::SecureSocketException(numBytes);
			}
			return (size_t)numBytes;
		}

		std::string SecureSocketObj::read(int max_size /*= 0*/) const
		{
			const int buf_len = (max_size > 0 && max_size < MAX_MESSAGE_SIZE) ? max_size : MAX_MESSAGE_SIZE - 1;

			std::string str_message;
			std::unique_ptr<char[]> rec_ptr{ std::make_unique<char[]>(buf_len) };

			int receive_byte = 0;
			
			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			do
			{
				while ((receive_byte = SSL_read(m_ssl, rec_ptr.get(), buf_len)) == -1)
				{
					if (callback_interupt &&
						callback_interupt(m_socket_ref.m_userdata_ptr))
						throw general::SecureSocketException(INTERRUPT_MSG);

					switch (auto err_code = SSL_get_error(m_ssl, receive_byte))
					{
					case SSL_ERROR_WANT_READ:
						break;
					case SSL_ERROR_ZERO_RETURN:
						SSL_shutdown(m_ssl);
#if _HAS_CXX17
						[[fallthrough]];
#endif
					default:
						throw general::SecureSocketException(err_code);
					}
				}

				if (receive_byte > 0)
				{
					std::move(rec_ptr.get(), rec_ptr.get() + receive_byte, std::back_inserter(str_message));
				}

				if (SSL_pending(m_ssl) == 0)
					break;

				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SecureSocketException(INTERRUPT_MSG);

				if (max_size > 0 && str_message.size() >= (size_t)max_size)
					break;

			} while (receive_byte > 0);

			return str_message;

		}

		size_t SecureSocketObj::read(std::vector<unsigned char>& message, int max_size /*= 0*/) const
		{
			auto received_str = read(max_size);
			std::move(received_str.begin(), received_str.end(), std::back_inserter(message));
			return message.size();
		}

		size_t SecureSocketObj::read(std::string& message, int max_size /*= 0*/) const
		{
			message = read(max_size);
			return message.size();
		}

		int SecureSocketObj::write(std::initializer_list<char> data_list) const
		{
			return write(data_list.begin(), (int)data_list.size());
		}

		int SecureSocketObj::write(const char* data, int data_size) const
		{
			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			int sendBytes{};
			while ((sendBytes = SSL_write(m_ssl, data, data_size)) == -1)
			{
				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SecureSocketException(INTERRUPT_MSG);

				switch (auto err_code = SSL_get_error(m_ssl, sendBytes))
				{
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(m_ssl);
#if _HAS_CXX17
					[[fallthrough]];
#endif
				default:
					throw general::SecureSocketException(err_code);
				}
			}
			return sendBytes;
		}

		int SecureSocketObj::write(const std::vector<unsigned char>& message) const
		{
			std::string strBuf(message.begin(), message.end());
			return write(strBuf.c_str(), (int)strBuf.size());
		}

		int SecureSocketObj::write(const std::string& message) const
		{
			return write(message.c_str(),
				static_cast<int>(message.size()));
		}
#endif // OPENSSL_SUPPORTED
	}
}
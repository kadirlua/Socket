#include "SecureServer.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"
#include "pcout.h"

namespace sdk {
	namespace application {

		using namespace network;

#if OPENSSL_SUPPORTED

		static inline int verify_callback(int preverify_ok, X509_STORE_CTX* x509_ctx)
		{
			X509* cert = X509_STORE_CTX_get_current_cert(x509_ctx);
			if (!cert)
				return 0;

			int err_code = X509_STORE_CTX_get_error(x509_ctx);
			int check;
			if (err_code != X509_V_OK)
			{
				return err_code;
			}

			auto subject_name = X509_get_subject_name(cert);
			if (!subject_name)
				return 0;

			char buf[6][256]{};
			printf("Certificate subject:\n");
			check = X509_NAME_get_text_by_NID(subject_name, NID_commonName, buf[0], sizeof(buf[0]));
			if (check > 0)
				printf(" - Common name: %s\n", buf[0]);
			check = X509_NAME_get_text_by_NID(subject_name, NID_organizationName, buf[1], sizeof(buf[1]));
			if (check > 0)
				printf(" - Organization name: %s\n", buf[1]);
			check = X509_NAME_get_text_by_NID(subject_name, NID_organizationalUnitName, buf[2], sizeof(buf[2]));
			if (check > 0)
				printf(" - Organizational unit name: %s\n", buf[2]);
			printf("Certificate issuer:\n");
			check = X509_NAME_get_text_by_NID(subject_name, NID_commonName, buf[3], sizeof(buf[3]));
			if (check > 0)
				printf(" - Common name: %s\n", buf[3]);
			check = X509_NAME_get_text_by_NID(subject_name, NID_organizationName, buf[4], sizeof(buf[4]));
			if (check > 0)
				printf(" - Organization name: %s\n", buf[4]);
			check = X509_NAME_get_text_by_NID(subject_name, NID_organizationalUnitName, buf[5], sizeof(buf[5]));
			if (check > 0)
				printf(" - Organizational unit name: %s\n", buf[5]);

			return preverify_ok;
		}

		static void thread_purging()
		{
			while (!purging_flag_)
			{
				std::unique_lock<std::mutex> lock_(vec_mutex_);
				vec_cv_.wait(lock_, []() { return !thread_vec_.empty() || purging_flag; });
				for (auto iter = thread_vec_.rbegin(); iter != thread_vec_.rend(); iter++)
				{
					if ((*iter)->isClosed())
					{
						thread_vec_.erase(iter.base() - 1);
						break;
					}
				}
			}
		}

		SecureServer::SecureServer(int port, network::protocol_type type , network::IpVersion ipVer) :
			listener_thread{ new std::thread{ &SecureServer::listener_thread_proc, this, port, type, ipVer} },
			purging_thread{ new std::thread{ &thread_purging } }
		{

		}

		SecureServer::~SecureServer()
		{
			vec_mutex_.lock();
			purging_flag_ = true;
			vec_mutex_.unlock();
			vec_cv_.notify_all();
		}

		void SecureServer::listener_thread_proc(int port_, network::protocol_type type , network::IpVersion ipVer )
		{
			try {
				m_socket_ptr = std::make_unique<SecureSocket>(port_, connection_method::server ,type, ipVer);
				SocketOption<SecureSocket> socketOpt{ *m_socket_ptr };
				socketOpt.setBlockingMode(1);	//non-blocking mode	
				const char* cert_file = "C:\\Program Files\\OpenSSL\\bin\\certificate.pem";
				const char* key_file = "C:\\Program Files\\OpenSSL\\bin\\key.key";
				//set certificate properties
				m_socket_ptr->loadVerifyLocations(cert_file, nullptr);
				/* Load the client's CA file location as well */
				//m_socket_ptr->loadClientCertificateList("client.pem");
				//m_socket_ptr->setCipherList("AES128-SHA");
				m_socket_ptr->loadCertificateFile(cert_file);
				m_socket_ptr->loadPrivateKeyFile(key_file);

				m_socket_ptr->setCallbackVerifyCertificate(SSL_VERIFY_PEER |
					/*SSL_VERIFY_CLIENT_ONCE |*/ SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);

				/* We accept only certificates signed only by the CA himself */
				//m_socket_ptr->setVerifyDepth(1);

				//bind and listen
				m_socket_ptr->bind();
				m_socket_ptr->listen(10);

				while (true) {

					try {
						SOCKET new_socket_id = m_socket_ptr->accept();
						
						auto ssl_obj = m_socket_ptr->createnewSocket(new_socket_id);
						ssl_obj->accept();

						std::unique_lock<std::mutex> lock_(vec_mutex_);
						thread_vec_.emplace_back(new WorkerThread<SecureSocketObj>(ssl_obj));
						lock_.unlock();
						vec_cv_.notify_one();
					}
					catch (const general::SocketException& ex)
					{
						(void)ex;
					}
				}
			}
			catch (const general::SecureSocketException& ex)
			{
				pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
			}
		}

		template<>
		void WorkerThread<SecureSocketObj>::handle_thread_proc(std::weak_ptr<SecureSocketObj> ssl_obj)
		{
			auto sharedSocketPtr = ssl_obj.lock();
			if (sharedSocketPtr)
			{
				try
				{
					std::string request_message;
					std::string response = "I hear You";

					while (true)
					{
						try {
							sharedSocketPtr->read(request_message);
							pcout{} << request_message << "\n";
							sharedSocketPtr->write(response);
						}
						catch (const general::SecureSocketException& ex)
						{
							pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
							break;
						}
					}
				}
				catch (const general::SecureSocketException& ex)
				{
					pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
				}
			}

			vec_mutex_.lock();
			close_flag = true;
			vec_mutex_.unlock();
			vec_cv_.notify_one();
		}
#endif // OPENSSL_SUPPORTED
	}
}
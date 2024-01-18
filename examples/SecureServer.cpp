﻿// MIT License

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

#include "SecureServer.h"
#include "general/SocketException.h"
#include "network/SocketOption.h"
#include "pcout.h"

#include <algorithm>

namespace sdk {
	namespace application {

		using namespace network;

#if OPENSSL_SUPPORTED

		namespace {
			// only for testing
			const char* const certFile = R"(C:\Program Files\OpenSSL\bin\certificate.pem)";
			const char* const keyFile = R"(C:\Program Files\OpenSSL\bin\key.key)";

			int verify_callback(int preverify_ok, X509_STORE_CTX* x509Ctx)
			{
				const auto* cert = X509_STORE_CTX_get_current_cert(x509Ctx);
				if (cert == nullptr) {
					return 0;
				}

				int const errCode = X509_STORE_CTX_get_error(x509Ctx);
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

				return preverify_ok;
			}

			void thread_purging()
			{
				while (!purging_flag_) {
					std::unique_lock<std::mutex> lock_(vec_mutex_);
					vec_cv_.wait(lock_, []() { return !thread_vec_.empty() || purging_flag; });
					thread_vec_.erase(std::remove_if(thread_vec_.begin(), thread_vec_.end(),
										  [](const auto& socketObj) {
											  return socketObj->isClosed();
										  }),
						thread_vec_.end());
				}
			}
		}

		SecureServer::SecureServer(int port, network::ProtocolType type, network::IpVersion ipVer) :
			listener_thread{ new std::thread{ &SecureServer::listener_thread_proc, this, port, type, ipVer } },
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

		void SecureServer::listener_thread_proc(int port_, network::ProtocolType type, network::IpVersion ipVer)
		{
			try {
				m_socket_ptr = std::make_unique<SSLSocket>(port_, ConnMethod::server, type, ipVer);
				const SocketOption<SSLSocket> socketOpt{ *m_socket_ptr };
				socketOpt.setBlockingMode(1); // non-blocking mode
				socketOpt.setReuseAddr(1);

				// set certificate properties
				m_socket_ptr->loadVerifyLocations(certFile, nullptr);
				/* Load the client's CA file location as well */
				// m_socket_ptr->loadClientCertificateList("client.pem");
				// m_socket_ptr->setCipherList("AES128-SHA");
				m_socket_ptr->loadCertificateFile(certFile);
				m_socket_ptr->loadPrivateKeyFile(keyFile);

				m_socket_ptr->setCallbackVerifyCertificate(SSL_VERIFY_PEER |
															   /*SSL_VERIFY_CLIENT_ONCE |*/ SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
					verify_callback);

				/* We accept only certificates signed only by the CA himself */
				// m_socket_ptr->setVerifyDepth(1);

				// bind and listen
				m_socket_ptr->bind();
				m_socket_ptr->listen(10);

				while (true) {

					try {
						SOCKET const new_socket_id = m_socket_ptr->accept();

						auto ssl_obj = m_socket_ptr->createNewSocket(new_socket_id);
						ssl_obj->accept();

						std::unique_lock<std::mutex> lock_(vec_mutex_);
						thread_vec_.emplace_back(new WorkerThread<SSLSocketDescriptor>(ssl_obj));
						lock_.unlock();
						vec_cv_.notify_one();
					}
					catch (const general::SocketException& ex) {
						(void)ex;
					}
				}
			}
			catch (const general::SSLSocketException& ex) {
				pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
			}
		}

		template <>
		void WorkerThread<SSLSocketDescriptor>::handle_thread_proc(const std::weak_ptr<SSLSocketDescriptor>& ssl_obj)
		{
			auto sharedSocketPtr = ssl_obj.lock();
			if (sharedSocketPtr) {
				try {
					std::string request_message;
					std::string const response = "I hear You";

					while (true) {
						try {
							sharedSocketPtr->read(request_message);
							pcout{} << request_message << "\n";
							sharedSocketPtr->write(response);
						}
						catch (const general::SSLSocketException& ex) {
							pcout{} << ex.getErrorCode() << ": " << ex.getErrorMsg() << "\n";
							break;
						}
					}
				}
				catch (const general::SSLSocketException& ex) {
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

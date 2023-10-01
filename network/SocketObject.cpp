#include "SocketObject.h"
#include "Socket.h"
#include "general/SocketException.h"
#include "SocketOption.h"

#include <memory>
#include <iterator>

namespace sdk {
	namespace network {

		SocketObject::SocketObject(SOCKET socketId, const Socket& socket_ref) noexcept :
			m_socket_id{ socketId },
			m_socket_ref{ socket_ref }
		{
		}

		SocketObject::~SocketObject()
		{
			shutdown(m_socket_id, SD_SEND);

			while (closesocket(m_socket_id) == SOCKET_ERROR) {
				auto err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK) {
					break;
				}
			}
		}

		std::string SocketObject::read(int max_size /*= 0*/) const
		{
			const int buf_len = (max_size > 0 && max_size < MAX_MESSAGE_SIZE) ? max_size : MAX_MESSAGE_SIZE - 1;

			std::string str_message;
			std::vector<char> dataVec;
			dataVec.reserve(buf_len);

			int receive_byte{};
			int iResult = 0;
			struct timeval tv = { 0, 0 };

			fd_set readFds{};
			fd_set exceptFds{};

			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			const SocketOption<SocketObject> socketOpt{ *this };

			/*if (socketOpt.getBytesAvailable() == 0)
				return str_message;*/

			do {
				while ((receive_byte = recv(m_socket_id, dataVec.data(), buf_len, 0)) == SOCKET_ERROR) {
					if (callback_interrupt &&
						callback_interrupt(m_socket_ref.m_userdata_ptr)) {
						throw general::SocketException(INTERRUPT_MSG);
					}

					switch (auto lasterror = WSAGetLastError()) {
					case WSAEWOULDBLOCK: {
						auto recvTimeout = socketOpt.getRecvTimeout();
						//	The default value of recieve timeout is 0.
						//	If a user decided to set timeout value, there is no problem at all.
						//	Otherwise, set the default value to an acceptable timeout value.
						if (recvTimeout.tv_sec == 0 &&
							recvTimeout.tv_usec == 0) {
							recvTimeout.tv_sec = 5;
						}

						FD_ZERO(&readFds);
						FD_SET(m_socket_id, &readFds);

						iResult = select((int)m_socket_id + 1, &readFds, nullptr, nullptr, &recvTimeout);
						if (iResult < 0) {
							throw general::SocketException(WSAGetLastError());
						}
						if (!FD_ISSET(m_socket_id, &readFds)) {
							return str_message;
						}
					} break;
					default:
						throw general::SocketException(lasterror);
					}
				}

				if (receive_byte == 0) {
					return str_message; // the connection is closed.
				}

				if (receive_byte > 0) {
					std::move(dataVec.begin(), dataVec.begin() + receive_byte,
						std::back_inserter(str_message));
				}

				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				if (max_size > 0 && str_message.size() >= (size_t)max_size) {
					break;
				}

				/*
				 *	Wait for a while to detect if we reached the end of the data
				 *	select will return immediately without waiting...
				 */

				FD_ZERO(&readFds);
				FD_SET(m_socket_id, &readFds);
				FD_ZERO(&exceptFds);
				FD_SET(m_socket_id, &exceptFds);
				iResult = select((int)m_socket_id + 1, &readFds, nullptr, &exceptFds, &tv);
				if (iResult < 0) {
					throw general::SocketException(WSAGetLastError());
				}
				if (!FD_ISSET(m_socket_id, &readFds) || FD_ISSET(m_socket_id, &exceptFds)) {
					break;
				}
			} while (receive_byte > 0);

			return str_message;
		}


		size_t SocketObject::read(char& msgByte) const
		{
			int const numBytes = recv(m_socket_id, &msgByte, 1, 0);
			if (numBytes < 0) {
				throw general::SocketException(WSAGetLastError());
			}
			return static_cast<size_t>(numBytes);
		}

		size_t SocketObject::read(std::vector<unsigned char>& message, int max_size /*= 0*/) const
		{
			const auto received_str = read(max_size);
			std::move(received_str.begin(), received_str.end(), std::back_inserter(message));
			return message.size();
		}

		size_t SocketObject::read(std::string& message, int max_size /*= 0*/) const
		{
			message = read(max_size);
			return message.size();
		}

		int SocketObject::write(std::initializer_list<char> data_list) const
		{
			return write(data_list.begin(), (int)data_list.size());
		}

		int SocketObject::write(const char* data, int data_size) const
		{
			int sendBytes = 0;

			const auto& callback_interrupt = m_socket_ref.m_callback_interrupt;

			while ((sendBytes = send(m_socket_id, data, data_size, 0)) == SOCKET_ERROR) {
				if (callback_interrupt &&
					callback_interrupt(m_socket_ref.m_userdata_ptr)) {
					throw general::SocketException(INTERRUPT_MSG);
				}

				switch (auto lasterror = WSAGetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				default:
					throw general::SocketException(lasterror);
				}
			}

			return sendBytes;
		}

		int SocketObject::write(const std::vector<unsigned char>& message) const
		{
			const std::string strBuf(message.begin(), message.end());
			return write(strBuf.c_str(), (int)strBuf.size());
		}

		int SocketObject::write(const std::string& message) const
		{
			return write(message.c_str(),
				static_cast<int>(message.size()));
		}
	}
}
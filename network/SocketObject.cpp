#include "SocketObject.h"
#include "Socket.h"
#include "general/SocketException.h"
#include "SocketOption.h"

#include <memory>
#include <iterator>

namespace sdk {
	namespace network {

		SocketObj::SocketObj(SOCKET socketid, const Socket& socket_ref) noexcept :
			m_socket_id{ socketid },
			m_socket_ref{ socket_ref }
		{

		}

		SocketObj::~SocketObj()
		{
			shutdown(m_socket_id, SD_SEND);

			while (closesocket(m_socket_id) == SOCKET_ERROR)
			{
				auto err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
					break;
			}
		}

		std::string SocketObj::read(int max_size /*= 0*/) const
		{
			const int buf_len = (max_size > 0 && max_size < MAX_MESSAGE_SIZE) ? max_size : MAX_MESSAGE_SIZE - 1;

			std::string str_message;
			std::unique_ptr<char[]> rec_ptr{ std::make_unique<char[]>(buf_len) };
			
			int receive_byte{};
			int iResult;
			struct timeval tv = { 0, 0 };

			fd_set readfds{}, exceptfds{};

			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			SocketOption<SocketObj> socketOpt{ *this };

			/*if (socketOpt.getBytesAvailable() == 0)
				return str_message;*/

			do {
				while ((receive_byte = recv(m_socket_id, rec_ptr.get(), buf_len, 0)) == SOCKET_ERROR)
				{
					if (callback_interupt &&
						callback_interupt(m_socket_ref.m_userdata_ptr))
						throw general::SocketException(INTERRUPT_MSG);

					switch (auto lasterror = WSAGetLastError())
					{
					case WSAEWOULDBLOCK:
					{
						auto recvTimeout = socketOpt.getRecvTimeout();
						//	The default value of recieve timeout is 0. 
						//	If an user decided to set timeout value, there is no problem at all. 
						//	Otherwise set the default value to an acceptable timeout value.
						if (recvTimeout.tv_sec == 0 && 
							recvTimeout.tv_usec == 0)
							recvTimeout.tv_sec = 5;

						FD_ZERO(&readfds);
						FD_SET(m_socket_id, &readfds);

						iResult = select((int)m_socket_id + 1, &readfds, nullptr, nullptr, &recvTimeout);
						if (iResult < 0)
							throw general::SocketException(WSAGetLastError());
						if (!FD_ISSET(m_socket_id, &readfds))
							return str_message;
					}
						break;
					default:
						throw general::SocketException(lasterror);
					}
				}

				if (receive_byte == 0)
					return str_message;	//the connection is closed.
				
				if (receive_byte > 0)
					std::move(rec_ptr.get(), rec_ptr.get() + receive_byte, std::back_inserter(str_message));

				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SocketException(INTERRUPT_MSG);

				if (max_size > 0 && str_message.size() >= (size_t)max_size)
					break;

				/*
				*	Wait for a while to detect if we reached the end of the data
				*	select will return immediately without waiting...
				*/

				FD_ZERO(&readfds);
				FD_SET(m_socket_id, &readfds);
				FD_ZERO(&exceptfds);
				FD_SET(m_socket_id, &exceptfds);
				iResult = select((int)m_socket_id + 1, &readfds, nullptr, &exceptfds, &tv);
				if (iResult < 0)
					throw general::SocketException(WSAGetLastError());
				if (!FD_ISSET(m_socket_id, &readfds) || FD_ISSET(m_socket_id, &exceptfds))
					break;

			} while (receive_byte > 0);

			return str_message;
		}


		size_t SocketObj::read(char& msgByte) const
		{
			int numBytes = recv(m_socket_id, &msgByte, 1, 0);
			if (numBytes < 0)
			{
				throw general::SocketException(WSAGetLastError());
			}
			return static_cast<size_t>(numBytes);
		}

		size_t SocketObj::read(std::vector<unsigned char>& message, int max_size /*= 0*/) const
		{
			auto received_str = read(max_size);
			std::move(received_str.begin(), received_str.end(), std::back_inserter(message));
			return message.size();
		}

		size_t SocketObj::read(std::string& message, int max_size /*= 0*/) const
		{
			message = read(max_size);
			return message.size();
		}

		int SocketObj::write(std::initializer_list<char> data_list) const
		{
			return write(data_list.begin(), (int)data_list.size());
		}

		int SocketObj::write(const char* data, int data_size) const
		{
			int sendBytes = 0;

			const auto& callback_interupt = m_socket_ref.m_callback_interrupt;

			while ((sendBytes = send(m_socket_id, data, data_size, 0)) == SOCKET_ERROR)
			{
				if (callback_interupt &&
					callback_interupt(m_socket_ref.m_userdata_ptr))
					throw general::SocketException(INTERRUPT_MSG);

				switch (auto lasterror = WSAGetLastError())
				{
				case WSAEWOULDBLOCK:
					break;
				default:
					throw general::SocketException(lasterror);
				}
			}

			return sendBytes;
		}

		int SocketObj::write(const std::vector<unsigned char>& message) const
		{
			std::string strBuf(message.begin(), message.end());
			return write(strBuf.c_str(), (int)strBuf.size());
		}

		int SocketObj::write(const std::string& message) const
		{
			return write(message.c_str(),
				static_cast<int>(message.size()));
		}
	}
}
#include "BaseException.h"

namespace sdk {
	namespace general {

		BaseException::BaseException() noexcept {

		}

		BaseException::BaseException(std::string&& err_msg) noexcept : 
			m_error_msg{ std::move(err_msg) }
		{

		}

		BaseException::BaseException(const std::string& err_msg) noexcept :
			m_error_msg{ err_msg }
		{
		
		}

		BaseException::~BaseException()
		{
		}

		BaseException::BaseException(const BaseException& other)
		{
			if (this != &other)
			{
				m_error_msg = other.m_error_msg;
			}

		}

		BaseException& BaseException::operator=(const BaseException& other)
		{
			if (this != &other)
			{
				m_error_msg = other.m_error_msg;
			}
			return *this;

		}

		BaseException::BaseException(BaseException&& other) noexcept
		{
			if (this != &other)
			{
				m_error_msg = std::move(other.m_error_msg);
			}

		}

		BaseException& BaseException::operator=(BaseException&& other) noexcept
		{
			if (this != &other)
			{
				m_error_msg = std::move(other.m_error_msg);
			}
			return *this;
		}
	}
}

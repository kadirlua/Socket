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

#include "BaseException.h"

namespace sdk {
	namespace general {

		BaseException::BaseException(std::string&& err_msg) noexcept : 
			m_error_msg{ std::move(err_msg) }
		{

		}

		BaseException::BaseException(const std::string& err_msg) noexcept :
			m_error_msg{ err_msg }
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

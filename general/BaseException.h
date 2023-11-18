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

#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include "ExceptionExport.h"
#include <string>

namespace sdk {
	namespace general {
		/*
		*	This class is common interface for exceptions.
		*	You can inherit your own exception classes.
		*	class myException : public BaseException{}.
		*/
		class EXCEPTION_API BaseException
		{
		public:
			BaseException() noexcept = default;
			BaseException(const std::string& err_msg) noexcept;
			BaseException(std::string&& err_msg) noexcept;
			virtual ~BaseException() = default;
			BaseException(const BaseException&);				//copy constructor
			BaseException& operator=(const BaseException&);		//copy assignment operator
			BaseException(BaseException&&) noexcept;						//move constructor
			BaseException& operator=(BaseException&&) noexcept;				//move assignment operator
			/*
			*	Gets an error message for detect which error occurs.
			*	returns: Error message.
			*	exception: This function never throws an exception.
			*/
			virtual const std::string& getErrorMsg() const noexcept { 
				return m_error_msg; 
			}

		protected:
			std::string m_error_msg;
		};
	}
}

#endif
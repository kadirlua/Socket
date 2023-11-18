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

#ifndef SOCKET_EXCEPTION_H_
#define SOCKET_EXCEPTION_H_

#include "BaseException.h"
#include "ExceptionExport.h"

namespace sdk {
    namespace general {

        /*
        *   These classes determines exception errors while using socket classes.
        */
        class EXCEPTION_API SocketException : public BaseException
        {
        public:
            SocketException(int err_code) noexcept;
            SocketException(int err_code, std::string&& err_msg) noexcept;
            SocketException(int err_code, const std::string& err_msg) noexcept;
            SocketException(const std::string& err_msg) noexcept;
            SocketException(std::string&& err_msg) noexcept;
            /*
            *	Gets an error code for detect which error occurs.
            *	returns: Error code.
            *	exception: This function never throws an exception.
            */
            int getErrorCode() const noexcept {
                return m_error_code;
            }
        protected:
            int m_error_code{};
            std::string GetWSALastErrorMessage() const noexcept;
        };

        class EXCEPTION_API SecureSocketException : public SocketException
        {
        public:
            SecureSocketException(int err_code) noexcept;
            SecureSocketException(int err_code, std::string&& err_msg) noexcept;
            SecureSocketException(int err_code, const std::string& err_msg) noexcept;
            SecureSocketException(const std::string& err_msg) noexcept;
            SecureSocketException(std::string&& err_msg) noexcept;
        };
    }
}

#endif
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

#include "SocketException.h"
#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <string.h>
#endif

#if OPENSSL_SUPPORTED
#include <openssl/err.h>
#endif // OPENSSL_SUPPORTED

namespace sdk{
    namespace general
    {
        SocketException::SocketException(int err_code) noexcept :
            m_error_code{ err_code }
        {
            m_error_msg = GetWSALastErrorMessage();
        }

        SocketException::SocketException(int err_code, std::string&& err_msg) noexcept :
            BaseException( std::move(err_msg) ),
            m_error_code{ err_code }
        {
        }

        SocketException::SocketException(int err_code, const std::string& err_msg) noexcept :
            BaseException(err_msg),
            m_error_code{ err_code }
        {
        }

        SocketException::SocketException(std::string&& err_msg) noexcept :
            BaseException(std::move(err_msg))
        {
        }

        SocketException::SocketException(const std::string& err_msg) noexcept :
            BaseException(err_msg)
        {
        }

        std::string SocketException::GetWSALastErrorMessage() const noexcept
        {
            std::string strErrMsg;
#ifdef _WIN32
            char* err_msg = nullptr;
            if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
                m_error_code,
                0,
                (LPSTR)&err_msg,
                0,
                nullptr) == 0U) {
				return strErrMsg;
			}

            strErrMsg = err_msg;
            LocalFree(err_msg);
#elif __linux__
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
            char strBuffer[1024]{};
            if (strerror_r(m_error_code, strBuffer, sizeof(strBuffer) * sizeof(char)) == 0)
                strErrMsg = strBuffer;
#else
            strErrMsg = strerror(m_error_code);
#endif
#endif
            return strErrMsg;
        }

        SecureSocketException::SecureSocketException(int err_code) noexcept :
            SocketException(err_code)
        {
#if OPENSSL_SUPPORTED
            int err{};
            while ((err = ERR_get_error()) != 0) {
                m_error_msg += ERR_error_string(err, nullptr);
            }
#endif // OPENSSL_SUPPORTED
        }

        SecureSocketException::SecureSocketException(int err_code, std::string&& err_msg) noexcept :
            SocketException(err_code, std::move(err_msg))
        {
        }

        SecureSocketException::SecureSocketException(int err_code, const std::string& err_msg) noexcept :
            SocketException(err_code, err_msg)
        {
        }

        SecureSocketException::SecureSocketException(std::string&& err_msg) noexcept :
            SocketException(std::move(err_msg))
        {
        }

        SecureSocketException::SecureSocketException(const std::string& err_msg) noexcept :
            SocketException(err_msg)
        {
        }

    }
}

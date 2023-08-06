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
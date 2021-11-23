#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include <string>

namespace sdk {
	namespace general {
		/*
		*	This class is common interface for exceptions.
		*	You can inherit your own exception classes.
		*	class myexception : public BaseException{}.
		*/
		class BaseException
		{
		public:
			BaseException() noexcept;
			BaseException(const std::string& err_msg) noexcept;
			BaseException(std::string&& err_msg) noexcept;
			virtual ~BaseException();
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
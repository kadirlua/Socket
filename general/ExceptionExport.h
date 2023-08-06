#ifndef EXCEPTION_EXPORT_H_
#define EXCEPTION_EXPORT_H_

/* Windows DLL export/import */
#if defined(_WIN32) && !defined(__GNUC__) && defined(EXCEPTION_COMPILE_DLL)
    #if EXCEPTION_DLL_EXPORT
        #define EXCEPTION_API __declspec(dllexport)
    #else
        #define EXCEPTION_API __declspec(dllimport)
    #endif    
#else    
    #if __GNUC__ >= 4
        #define EXCEPTION_API __attribute__ ((visibility ("default")))
    #else
        #define EXCEPTION_API
    #endif
#endif

#endif
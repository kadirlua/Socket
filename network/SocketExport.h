#ifndef SOCKET_EXPORT_H_
#define SOCKET_EXPORT_H_

/* Windows DLL export/import */
#if defined(_WIN32) && !defined(__GNUC__) && defined(SOCKET_COMPILE_DLL)
    #if SOCKET_DLL_EXPORT
        #define SOCKET_API __declspec(dllexport)
    #else
        #define SOCKET_API __declspec(dllimport)
    #endif    
#else    
    #if __GNUC__ >= 4
        #define SOCKET_API __attribute__ ((visibility ("default")))
    #else
        #define SOCKET_API
    #endif
#endif

#endif
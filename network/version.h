#ifndef VERSION_H_
#define VERSION_H_

#define DO_MAKE_STR(x) #x
#define MAKE_STR(x) DO_MAKE_STR(x)
#define VERSION_STR(major, minor, patch) MAKE_STR(major) "." \
        MAKE_STR(minor) "." \
        MAKE_STR(patch)

#endif
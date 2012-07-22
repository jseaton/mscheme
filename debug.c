#include "debug.h"
#include <stdio.h>

#if defined(NDEBUG)
#else
void pmesg(char* format, ...) {
#ifdef NDEBUG

#else
        va_list args;

        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
#endif
}
#endif

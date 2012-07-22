#ifndef DEBUG_H
#define DEBUG_H
#include <stdarg.h>

#if defined(NDEBUG)
#define pmesg(format, args...) ((void)0)
#else
void pmesg(char *format, ...);
#endif
#endif

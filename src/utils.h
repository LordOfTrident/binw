#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdio.h>  /* stderr, fprintf */
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <assert.h> /* assert */

#define UNREACHABLE() assert(0 && "Unreachable")

void fatal(const char *p_fmt, ...);

#endif

#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#ifdef USE_DEBUG
void _cff_debug_head(const char * f, int lno) {
    fprintf(stderr, "[%s %d] ", f, lno);
}
void _cff_debug(const char * fmt, ...) {
    va_list ap;
    char buff[1024];

    va_start(ap, fmt);
    vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s\n", buff);
}
void _cff_debug_err(const char * fmt, ...) {
    va_list ap;
    char buff[1024];

    va_start(ap, fmt);
    vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);

    perror(buff);
}
#endif

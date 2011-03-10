#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <printf.h>

static int printf_arginfo_bitstring(const struct printf_info * info,
        size_t n,
        int * argtypes) {
    if (n > 0)
        argtypes[0] = PA_STRING;
    return 1;
}

static int printf_output_bitstring(FILE * stream,
        const struct printf_info * info,
        const void * const * args) {
    const char * s, * ps;
    char * buf, * pbuf;
    char c;
    size_t len, buflen;

    // "a..."
    // <01100001, ...>
    s = *(const char **)(args[0]);
    if (info->width > 0)
        len = info->width;
    else
        len = strlen(s);
    buflen = len * 10;
    buf = (char *)malloc(buflen);
    if (!buf)
        return -1;

    ps = s;
    pbuf = buf + 1;

    buf[0] = '<';
    while (len--) {
        c = *ps++;
        if (pbuf != buf + 1) {
            *pbuf++ = ',';
            *pbuf++ = ' ';
        }
        *pbuf++ = (c >> 7) ? '1' : '0';
        *pbuf++ = (c & 64) ? '1' : '0';
        *pbuf++ = (c & 32) ? '1' : '0';
        *pbuf++ = (c & 16) ? '1' : '0';
        *pbuf++ = (c & 8) ? '1' : '0';
        *pbuf++ = (c & 4) ? '1' : '0';
        *pbuf++ = (c & 2) ? '1' : '0';
        *pbuf++ = (c & 1) ? '1' : '0';
    }
    buf[buflen - 1] = '>';

    len = fwrite((const void *)buf, sizeof(const char), buflen, stream);
    free(buf);
    
    return (len == buflen) ? buflen : -1;
}

int extend_printf() {
    if (register_printf_function('B', printf_output_bitstring,
                printf_arginfo_bitstring))
        return 0;
    return 1;
}

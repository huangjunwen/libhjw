#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "scgi_parser.h"

int on_header_name(const char * p, void * d) {
    *((const char **)d) = p;
    return 0;
}

int on_header_name_end(const char * p, void * d) {
    const char * b;
    b = *((const char **)d);
    printf("header: <%.*s>, ", p - b, b);
    return 0;
}

int on_header_val(const char * p, void * d) {
    *((const char **)d) = p;
    return 0;
}

int on_header_val_end(const char * p, void * d) {
    const char * b;
    b = *((const char **)d);
    printf("val: <%.*s>\n", p - b, b);
    return 0;
}

int on_headers_end(const char * p, void * d) {
    printf("body: <%s\n>", p + 1);
    return 0;
}

scgi_parser_callback_t callbacks = {
    .on_header_name = on_header_name,
    .on_header_name_end = on_header_name_end,
    .on_header_val = on_header_val,
    .on_header_val_end = on_header_val_end,
    .on_headers_end = on_headers_end
};

#define BUF_LEN (4096)

int main() {
    int len;
    char buf[BUF_LEN];
    const char * marker;
    scgi_parser_t parser;
    scgi_parser_err_t err;
    
    marker = NULL;
    scgi_parser_init(&parser, &callbacks, &marker);

    for (;;) {
        len = read(0, buf, BUF_LEN);       
        if (len < 0) {
            fprintf(stderr, "read err: %s\n", strerror(errno));
            return 1;
        }
        else if (len == 0) {
            fprintf(stderr, "eof before finish a request\n");
            return 1;
        }

        err = scgi_parser_run(&parser, buf, len);
        if (err == E_OK)
            break;
        else if (err == E_MORE_DATA)
            continue;
        else {
            fprintf(stderr, "parse err: %d\n", err);
            return 1;
        }

    }
    return 0;
}

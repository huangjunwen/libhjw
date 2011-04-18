#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "scgi.h"

int on_hdr_name(const char * p, int len, void * d) {
    printf("header: <%.*s>, ", len, p);
    return 0;
}

int on_hdr_val(const char * p, int len, void * d) {
    printf("val: <%.*s>\n", len, p);
    return 0;
}

scgi_req_cb_t cb = {
    .on_hdr_name = on_hdr_name,
    .on_hdr_val = on_hdr_val
};

int main() {
    int r;

    r = read_scgi_req(0, &cb, 0);
    printf("get scgi request return %d\n", r);

    printf("=============\n");
    char buf[1024];
    buf[1023] = '\0';

    r = fread(buf, 1, 1023, stdin);
    printf("%d: %s", r, buf);
    return 0;
}

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#define MAX_NS_LEN (16384)                  // 16K
#define HEADERS_BUF_CAP (MAX_NS_LEN + (4096))

typedef struct scgi_req_t {
    /* headers buffer, must large enough to hold MAX_NS_LEN
     * and other overheads
     */
    char hbuf[HEADERS_BUF_CAP];

    /* headers' starting address and its length
     * (netstring's len) 
     */
    int headers_len;
    char * headers;

    /* body */
    int body_len;
    char * body0;
    int body0_len;

} scgi_req_t;


int parse_req(int fd, scgi_req_t * req, int * err) {
    char * end, * p;
    size_t r;
    ssize_t n;

    req->hbuf[HEADERS_BUF_CAP - 1] = '\0';
    end = req->hbuf;
    r = HEADERS_BUF_CAP - 1;

#define READ_MORE_HEAD() { \
    if ((n = read(fd, (void *)end, r)) <= 0) return -1; \
    end += n; \
    r -= n; }

    // get headers' start address and its len
    while (1) {
        READ_MORE_HEAD();
        if ((p = strchr(req->hbuf, ':'))) {
            *p = '\0';
            req->headers = p + 1;
            req->headers_len = atoi(req->hbuf);
            if (req->headers_len <= 0 || req->headers_len > MAX_NS_LEN)
                return -2;
            break;
        }
    }

    // read headers
    p = req->headers + req->headers_len;
    while (p > end)
        READ_MORE_HEAD();

#undef READ_MORE_HEAD

    // get body len
    if (p < end) {
        req->body0 = p;
        req->body0_len = end - p;
    } else {
        req->body0 = 0;
        req->body0_len = 0;
    }
    req->body_len = atoi(req->headers + 15);       // len("CONTENT_LENGTH\0") == 15
    if (req->body_len <= 0)
        return -2;

    return 0;
}

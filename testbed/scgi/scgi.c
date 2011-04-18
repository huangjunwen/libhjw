#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "scgi.h"

#define MAX_HDR_SZ (64 * 1024)
#define HDR_BUF_SZ (MAX_HDR_SZ + 4096)

#define RETURN_ERR return -(__LINE__)

static int read_all(int fd, char * buf, int sz) {
    int r, n;
    char * p;

    r = sz;
    p = buf;
    while (r) {
        n = read(fd, p, r);
        if (n < 0) {
            if (errno != EINTR)
                RETURN_ERR;
        }
        else if (n == 0) {
            RETURN_ERR;
        }
        else {
            r -= n;
            p += n;
        }
    }

    return sz;
}

int read_scgi_req(int fd, scgi_req_cb_t * cb, void * user_data) {

    int r, n, hdr_len;
    char c;
    char * p, * e;
    char hbuff[HDR_BUF_SZ];

    /* first read
     *  net string's length's len >= 1 and < 10
     *  len('CONTENT_LENGTH') == 14
     * so first read 16 bytes, this guarantee
     *  1. we can get the header's length
     *  2. we doesn't read anything in body
     */
    n = read_all(fd, hbuff, 16);
    if (n < 0)
        RETURN_ERR;

    e = hbuff + n;
    p = hbuff;
    hdr_len = 0;
    do {
        if (hdr_len > MAX_HDR_SZ)
            RETURN_ERR;

        c = *p++;
        if (c == ':')
            break;

        if (!isdigit(c))
            RETURN_ERR;
        hdr_len *= 10;
        hdr_len += (c - '0');
    } while (p < e);

    // double check
    if (c != ':')
        RETURN_ERR;
    // now p pointer the one byte after ':'

    // second read
    // read to the end of headers (include the 
    // ending ',')
    n = read_all(fd, e, hdr_len + 1 - (e - p));
    if (n < 0)
        RETURN_ERR;

    e += n;
    *e = '\0';          // guard for strlen
    r = hdr_len;

    // callbacks
    for (;;) {
        // name
        n = strlen(p);
        r -= n + 1;
        if (r <= 0)
            RETURN_ERR;
        if ((cb->on_hdr_name)(p, n, user_data) < 0)
            RETURN_ERR;
        p += n + 1;

        // val
        n = strlen(p);
        r -= n + 1;
        if (r < 0)
            RETURN_ERR;
        if ((cb->on_hdr_val)(p, n, user_data) < 0)
            RETURN_ERR;
        p += n + 1;

        // end
        if (r == 0) {
            if (*p != ',')
                RETURN_ERR;
            break;
        }
    }

    return 0;

}

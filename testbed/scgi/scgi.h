#ifndef _SCGI_H_
#define _SCGI_H_

typedef struct {
    /* return < 0 if callback encouter an error
     */
    int (* on_hdr_name)(const char *, int, void *);

    int (* on_hdr_val)(const char *, int, void *);

} scgi_req_cb_t;

int read_scgi_req(int fd, scgi_req_cb_t * cb, void * user_data);

#endif


/* include the last '\r\n' before body */
#define MAX_HEAD_SIZE (32 * 1024)

typedef http_req_parser_t {
    char head_buff[MAX_HEAD_SIZE];

    /* point to the beginning of current request */
    char * pcurr;

    /* [pb, pe) is the last read content */
    char * pb;
    char * pe;
    char * mark;

    int cs;

} req_parser_t;

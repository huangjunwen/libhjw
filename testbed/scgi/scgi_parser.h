#ifndef _SCGI_PARSER_H_
#define _SCGI_PARSER_H_

typedef struct {
    /* return < 0 if callback encouter an error
     */

    // call at the start of a header name
    int (* on_header_name)(const char *, void *);

    // call at the end ('\0') of a header name
    int (* on_header_name_end)(const char *, void *);

    // similar to header name
    int (* on_header_val)(const char *, void *);
    int (* on_header_val_end)(const char *, void *);

    // call at the end (',') of all headers
    int (* on_headers_end)(const char *, void *);

} scgi_parser_callback_t;


typedef struct {
    enum {
        NS_LEN,
        NS_LEN_END,
        HEADER_NAME,
        HEADER_NAME_END,
        HEADER_VAL,
        HEADER_VAL_END,
        HEADERS_END
    } status;
    int header_length;
    int header_remain;
    scgi_parser_callback_t * callbacks;
    void * user_data;

} scgi_parser_t;


typedef enum {
    E_OK = 0,           // parse end success
    E_MORE_DATA,        // need more data
    E_CB_ERR,
    E_NS_LEN,
    E_NS_STR,
    E_UNKNOEN_ERR
} scgi_parser_err_t;

void scgi_parser_init(scgi_parser_t * parser, scgi_parser_callback_t * callbacks, 
        void * user_data);

scgi_parser_err_t scgi_parser_run(scgi_parser_t * parser, const char * buff, 
        int sz);
#endif

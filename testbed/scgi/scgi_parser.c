#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "scgi_parser.h"


void scgi_parser_init(scgi_parser_t * parser, scgi_parser_callback_t * callbacks, 
        void * user_data) {
    
    parser->status = NS_LEN;
    parser->header_length = 0;
    parser->header_remain = 0;
    parser->callbacks = callbacks;
    parser->user_data = user_data;
}

scgi_parser_err_t scgi_parser_run(scgi_parser_t * parser, const char * buff, 
        int sz) {

    char c;
    int header_remain;
    const char * p, * end;
    scgi_parser_callback_t * cb;

    p = buff;
    end = buff + sz;
    header_remain = parser->header_remain;
    cb = parser->callbacks;

reenter:

    switch (parser->status) {

        // In status NS_LEN
        // p should point to net string's len component (include ':')
        case NS_LEN:

            while (p < end) {
                c = *p++;
                if (c == ':') {
                    header_remain = parser->header_length;
                    parser->status = NS_LEN_END;
                    goto reenter;
                }

                if (!isdigit(c))
                    return E_NS_LEN;
                // TODO: need to check header length limit
                parser->header_length *= 10;
                parser->header_length += (c - '0');
            }

            // buff end at net string len -> "...89"
            goto buff_exhaust;

        // In status NS_LEN_END
        // p should point to a char after ':'
        case NS_LEN_END:

            if (p < end) {
                if (cb->on_header_name(p, parser->user_data) < 0)
                    return E_CB_ERR;
                parser->status = HEADER_NAME;
                goto reenter;
            }

            // buff end at ':' -> "...89:"
            goto buff_exhaust;

        // In status HEADER_NAME
        // p should point to any char in header name(include '\0')
        case HEADER_NAME:

            while (p < end) {
                if (--header_remain < 0)
                    return E_NS_STR;

                if (*p++)
                    continue;
                if (cb->on_header_name_end(p - 1, parser->user_data) < 0)
                    return E_CB_ERR;
                parser->status = HEADER_NAME_END;
                goto reenter;
            }

            // buff end at header name -> "...89:CONTENT_"
            goto buff_exhaust;

        // In status HEADER_NAME_END
        // p should point to a char after '\0'
        case HEADER_NAME_END:

            if (p < end) {
                if (cb->on_header_val(p, parser->user_data) < 0)
                    return E_CB_ERR;
                parser->status = HEADER_VAL;
                goto reenter;
            }

            // buff end at header name end -> "...89:CONTENT_LENGTH\0"
            goto buff_exhaust;
            
        // In status HEADER_VAL
        // p should point to any char in header val(include '\0')
        case HEADER_VAL:

            while (p < end) {
                if (--header_remain < 0)
                    return E_NS_STR;

                if (*p++)
                    continue;
                if (cb->on_header_val_end(p - 1, parser->user_data) < 0)
                    return E_CB_ERR;
                parser->status = HEADER_VAL_END;
                goto reenter;
            }

            // buff end at header val -> "...89:CONTENT_LENGTH\042"
            goto buff_exhaust;

        // In status HEADER_VAL_END
        // p should point to a char after '\0'
        case HEADER_VAL_END:

            if (p < end) {
                // net string end
                if (header_remain == 0) {
                    if (*p != ',')
                        return E_NS_STR;

                    if (cb->on_headers_end(p, parser->user_data) < 0)
                        return E_CB_ERR;
                    parser->status = HEADERS_END;
                    return E_OK;
                }

                // another header
                if (cb->on_header_name(p, parser->user_data) < 0)
                    return E_CB_ERR;
                parser->status = HEADER_NAME;
                goto reenter;
            }

            // buff end at header val end -> "...89:CONTENT_LENGTH\042\0"
            goto buff_exhaust;

        case HEADERS_END:
            return E_OK;

        default:
            return E_UNKNOEN_ERR;

    }

buff_exhaust:

    parser->header_remain = header_remain;
    return E_MORE_DATA;

}

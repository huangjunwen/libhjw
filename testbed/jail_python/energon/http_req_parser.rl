%%{

machine http_req_parser;

variable p parser->pb;
variable pe parser->pe;
variable cs parser->cs;

# actions
action mark {
    parser->mark = fpc;
}

action meth_get {
}

action meth_post {
}

action meth_other {
}

action uri {
}

action ver_major {
}

action ver_minor {
}

action field_name {
}

action field_val {
}

action got_err {
}

# rules

OCTET           = any;
CHAR            = ascii; 
UPALPHA         = upper;
LOALPHA         = lower;
ALPHA           = UPALPHA | LOALPHA;
DIGIT           = digit;
CTL             = cntrl | 127;
CR              = '\r';
LF              = '\n';
SP              = ' ';
HT              = '\t';
QUOTE           = '"';

CRLF            = CR LF;
LWS             = CRLF? ( SP | HT )+;
TEXT            = ( OCTET - CTL ) | LWS;
HEX             = xdigit;

SEPARATORS      = '(' | ')' | '<' | '>' | '@'
                | ',' | ';' | ':' | '\\' | QUOTE
                | '/' | '[' | ']' | '?' | '='
                | '{' | '}' | SP | HT;

TOKEN           = ( ( CHAR - CTL ) - SEPARATORS )+;

QDTEXT          = TEXT - '"';
QUOTED_PAIR     = '\\' CHAR;
QUOTED_STRING   = '"' ( QDTEXT | QUOTED_PAIR )* '"';

METH            = ( "GET" %meth_get 
                |  "POST" %meth_post
                | TOKEN %meth_other ) >mark;

URI_CHAR        = alnum | "#" | ":" | "?" | ";" | "@" | "&"
                | "=" | "+" | "$" | "," | "/" | "-" | "."
                | "$" | "_" | "!" | "~" | "*" | "'" | "("
                | ")" | "%";

REQ_URI         = ( "*" | URI_CHAR* ) >mark %uri;

# 1 digit is enough for quite a long time
HTTP_VERSION    = "HTTP/" DIGIT %ver_major "." DIGIT %ver_minor;


REQ_LINE        = METH SP REQ_URI SP HTTP_VERSION CRLF;

FIELD_SEP       = ":" ( SP | HT )*;

# field-content  = <the OCTETs making up the field-value
#                  and consisting of either *TEXT or combinations
#                  of token, separators, and quoted-string>
FIELD_VALUE     = ( ( OCTET - CTL - QUOTE ) | LWS | QUOTED_STRING )+;

MESSAGE_HEADER  = TOKEN >mark %field_name FIELD_SEP FIELD_VALUE >mark %field_val;

HTTP_REQ_HEAD   := REQ_LINE CRLF ( MESSAGE_HEADER CRLF )* CRLF $!got_err;

}%%

#include <unistd.h>
#include <string.h>
#include "http_req_parser.h"

%% write data;

void req_parser_init(req_parser_t * parser) {
    parser->pcurr = parser->pb = parser->pe = parser->head_buff;
    parser->mark = 0;
    %% write init;
}

void req_parser_clear(req_parser_t * parser) {

}

int req_parser_feed(int fd, req_parser_t * parser) {
    ssize_t n;
    off_t off;
    char * be;

    be = parser->head_buff + MAX_HEAD_SIZE;
    if (parser->pe >= be) {
        if (parser->pcurr == parser->head_buff)
            return -1;  // full

        /* there is some room before */
        memmove(parser->head_buff, parser->pcurr, 
                parser->pe - parser->pcurr);
        l = parser->pcurr - parser->head_buff;
        parser->pb -= off;
        parser->pe -= off;
        parser->mark -= parser->mark ? off : 0;
    }

    for(;;) {
        n = read(fd, parser->pe, be - parser->pe);
        if (n < 0) {
            if (n == EINTR)
                continue;
            return -2;
        }
        break;
    }

    parser->pb = parser->pe;
    parser->pe += n;
    return n;
}

int req_parser_exec(int fd, req_parser_t * parser) {
    ssize_t n;
    n = req_parser_feed(fd);
    if (n <= 0)
        return n;

    for (;;) {
        %% write exec;
    }
    
}

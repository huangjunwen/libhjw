%%{

machine http;

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
HT              = 9;
QUOTE           = '"';

CRLF            = CR LF;
LWS             = CRLF? ( SP | HT )+;
#TEXT            = ( OCTET - CTL ) | LWS
TEXT            = OCTET - CTL;
HEX             = xdigit;

separators      = '(' | ')' | '<' | '>' | '@'
                    | ',' | ';' | ':' | '\\' | QUOTE
                    | '/' | '[' | ']' | '?' | '='
                    | '{' | '}' | SP | HT;
token           = ( ( CHAR - CTL ) - separators )+;

qdtext          = TEXT - '"';
quoted_pair     = '\\' CHAR;
quoted_string   = '"' ( qdtext | quoted_pair )* '"';

}%%

#include <stdio.h>

action mark {
    mark = p;
}

action get_string {
    printf("%*.s\n", p - mark, p);
}

{%%

    machine http;

    main :=  quoted-string >mark %get_string;

}%%

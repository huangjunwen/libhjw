#ifndef _CFF_STR_UTIL_H_
#define _CFF_STR_UTIL_H_

typedef struct ref_str_t {
    unsigned int ref_count;             // ref count of this NULL-term string
    unsigned int length;                // length of the string (not include '\0')
    char str[1];                        // dyn-length struct
} ref_str_t;


// one should call ref_str_decref for each make_ref_str and ref_str_incref
const char * make_ref_str(const char * s);
void ref_str_incref(const char * rs);
void ref_str_decref(const char * rs);

#endif // _CFF_STR_UTIL_H_

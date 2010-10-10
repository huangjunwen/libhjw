#include <string.h>
#include "str_util.h"

static const size_t REF_STR_BASE_SIZE = (size_t)(((ref_str_t *)0)->str);

const char * make_ref_str(const char * s) {
    size_t len = strlen(s);
    ref_str_t * ref_str = (ref_str_t *)malloc(REF_STR_BASE_SIZE + len + 1);
    ref_str->ref_count = 1;                  // owned by the caller
    ref_str->length = len;
    strcpy(ref_str->str, s);
    return ref_str->str;
}

void ref_str_incref(const char * rs) {
    ++(((ref_str_t *)(rs - REF_STR_BASE_SIZE))->ref_count);
}

void ref_str_decref(const char * rs) {
    ref_str_t * ref_str = (ref_str_t *)(rs - REF_STR_BASE_SIZE);
    if (--ref_str->ref_count <= 0)
        free(ref_str);
}

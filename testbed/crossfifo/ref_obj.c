#include <string.h>
#include "ref_obj.h"

static const unsigned int REF_OBJ_BASE_SIZE = (unsigned int)(((ref_obj_t *)0)->obj);

// malloc space for a ref_obj
void * malloc_ref_obj(unsigned int size) {
    if (size < 1)
        return NULL;
    ref_obj_t * ro = (ref_obj_t *)malloc(REF_OBJ_BASE_SIZE + size);
    if (!ro)
        return NULL;
    ro->ref_count = 1;
    ro->obj_size = size;
    return (void *)ro->obj;
}

void ref_obj_incref(const void * obj) {
    ++(((ref_obj_t *)(obj - REF_OBJ_BASE_SIZE))->ref_count);
}

void ref_obj_decref(const void * obj) {
    ref_obj_t * ro = (ref_obj_t *)(obj - REF_OBJ_BASE_SIZE);
    if (--ro->ref_count <= 0)
        free(ro);
}

const char * make_ref_str(const char * s) {
    unsigned int len = strlen(s);
    char * ret = (char *)malloc_ref_obj(strlen(s) + 1);
    if (!ret)
        return NULL;
    strcpy(ret, s);
    return ret;
}


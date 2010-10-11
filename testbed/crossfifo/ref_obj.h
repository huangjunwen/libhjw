#ifndef _CFF_REF_OBJ_H_
#define _CFF_REF_OBJ_H_

typedef struct ref_obj_t {
    unsigned int ref_count;             // ref count of this NULL-term string
    unsigned int obj_size;              // whole ref_obj_t size
    char obj[1];                        // dyn-length struct
} ref_obj_t;

// !!! notes: be care not to make a circular reference

// malloc space for a ref_obj
void * malloc_ref_obj(unsigned int size);

// increase reference count
void ref_obj_incref(const void * obj);

// one should call 'ref_obj_decref' for each 'malloc_ref_obj' and 
// 'ref_obj_incref'
void ref_obj_decref(const void * obj);

// make a 'referenced' copy of string s
const char * make_ref_str(const char * s);

#endif // _CFF_REF_OBJ_H_

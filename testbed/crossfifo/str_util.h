#ifndef _CFF_STR_UTIL_H_
#define _CFF_STR_UTIL_H_

typedef struct ref_str_t {
    unsigned int ref_count;             // ref count of this NULL-term string
    unsigned int length;                // length of the string (not include '\0')
    struct ref_str_t * next;
    struct ref_str_t * prev;
    char str[1];                        // dyn-length struct
} ref_str_t;


typedef struct ref_str_pool_t {
    struct ref_str_t head;              // head is '\0'
} ref_str_pool_t;

void ref_str_pool_init(ref_str_pool_t * pool);
void ref_str_pool_destory(ref_str_pool_t * pool);

// one should call ref_str_free for each ref_str_add and ref_str_dup
const char * ref_str_add(ref_str_pool_t * pool, const char * str);
const char * ref_str_dup(const char * ref_str);
void ref_str_free(const char * ref_str);

#endif // _CFF_STR_UTIL_H_

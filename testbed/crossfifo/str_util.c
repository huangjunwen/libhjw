#include <string.h>
#include <assert.h>
#include "str_util.h"

void ref_str_pool_init(ref_str_pool_t * pool) {
    ref_str_t * head = &pool->head;
    head->ref_count = 1;                // owned by the pool itself
    head->length = 0;
    head->next = head->prev = head;
    head->str[0] = '\0';
}

void ref_str_pool_destory(ref_str_pool_t * pool) {
    ref_str_t * head = &pool->head;
    // there should be only one left
    assert(head->next == head && head->ref_count == 1);
}

static const size_t REF_STR_BASE_SIZE = (size_t)(((ref_str_t *)0)->str);

const char * ref_str_add(ref_str_pool_t * pool, const char * str) {
    if (!str[0]) {
        ++pool->head.ref_count;
        return &(pool->head.str);
    }
    size_t len = strlen(str);
    ref_str_t * rs = (ref_str_t *)malloc(REF_STR_BASE_SIZE + len + 1);
    rs->ref_count = 1;                  // owned by the caller
    rs->length = len;
    rs->next = pool->head.next;         // link
    rs->prev = &pool->head;
    pool->head.next->prev = rs;
    pool->head.next = rs;
    strcpy(rs->str, str);
    return rs->str;
}

const char * ref_str_dup(const char * ref_str) {
    ++((ref_str_t *)(ref_str - REF_STR_BASE_SIZE)->ref_count);
    return ref_str;
}

void ref_str_free(const char * ref_str) {
    ref_str_t * rs = (ref_str_t *)(ref_str - REF_STR_BASE_SIZE);
    if (--rs->ref_count <= 0) {
        assert(ref_str[0]);             // must not be the head
        rs->next->prev = rs->prev;      // unlink
        rs->prev->next = rs->next;
        free(rs);
    }
}

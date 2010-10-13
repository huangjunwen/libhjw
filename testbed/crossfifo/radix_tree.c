#include <assert.h>
#include <string.h>
#include "str_util.h"
#include "radix_tree.h"

static const unsigned char _bit_mask[8] = {128,64,32,16,8,4,2,1};

static unsigned char _get_bit(const char * s, unsigned int len, int bit_idx) {
    return (bit_idx < 0 || bit_idx >= len << 3) ? 0 :
        (s[bit_idx >> 3] & _bit_mask[bit_idx & 7]);
}

// call this only when the two strings are different, it doesn't check
// end bound
static unsigned int _diff_bit_idx(const char * s1, const char * s2) {
    const char * p1, * p2;
    char c;
    unsigned int i, j;

    p1 = s1;
    p2 = s2;
    i = 0;
    while (*p1 == *p2) {
        ++i;
        ++p1;
        ++p2;
    }
    i <<= 3;                    // *= 8

    c = (*p1) ^ (*p2);
    while ((c & 128) == 0) {
        c <<= 1;
        ++i;
    }
    return i;
}

kv_list_t * kv_list_create(unsigned int elem_size) {
    if (elem_size < sizeof(kv_t))
        return NULL;
    kv_list_t * ret = (kv_list_t *)malloc(sizeof(kv_list_t));
    if (!ret)
        return NULL;
    kv_t * head = &ret->head;
    head->next = head->prev = head;
    ret->length = 0;
    ret->elem_size = elem_size;
    return ret;
}

kv_t * kv_list_insert(kv_list_t * list, const char * key, void * val) {
    kv_t * ret = (kv_t *)malloc(list->elem_size);
    if (!ret)
        return NULL;
    ref_str_incref(key);
    ret->key = key;
    ret->val = val;
    ret->next = list->head.next;
    ret->prev = &list->head;
    list->head.next->prev = ret;
    list->head.next = ret;
    ++list->length;
    return ret;
}

void kv_list_remove(kv_list_t * list, kv_t * kv) {
    ref_str_decref(kv->key);
    kv->prev->next = kv->next;
    kv->next->prev = kv->prev;
    free(kv);
    --list->length;
}

void kv_list_destory(kv_list_t * list) {
    while (list->length)
        kv_list_remove(list, list->head.next);
} 

radix_tree_t * radix_tree_create() {
    radix_tree_t * ret = (radix_tree_t *)malloc(sizeof(radix_tree_t));
    if (!ret)
        return NULL;
    ret->node_list = kv_list_create(sizeof(rt_node_t));
    if (!ret->node_list) {
        free(ret);
        return NULL;
    }
    ret->root = NULL;
    return ret;
}

void radix_tree_destory(radix_tree_t * tree) {
}

rt_node_t * radix_tree_find(radix_tree_t * tree, const char * key) {
    rt_node_t * p;  // parent
    rt_node_t * c;  // child
    
    // ...

    unsigned int klen = strlen(key);
    while (c->bit_idx > p->bit_idx) {
        p = c;
        c = _get_bit(key, klen, c->bit_idx) ? c->right : c->left;
    }
    if (strncmp(key, c->kv.key))
        return NULL;
    return c;
}

rt_node_t * radix_tree_insert(struct radix_tree_t * tree, 
        const char * key, 
        void * val) {
    rt_node_t * p;  // parent
    rt_node_t * c;  // child
    
    // ...

    unsigned int klen = strlen(key);
    while (c->bit_idx > p->bit_idx) {
        p = c;
        c = _get_bit(key, klen, c->bit_idx) ? c->right : c->left;
    }
    if (strcmp(key, c->kv.key))
        return NULL;
    return c;
}

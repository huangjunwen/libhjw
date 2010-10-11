#include <assert.h>
#include <string.h>
#include "ref_obj.h"
#include "radix_tree.h"

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
    ref_obj_incref(key);
    ret->key = key;
    ref_obj_incref(val);
    ret->val = val;
    ret->next = list->head.next;
    ret->prev = &list->head;
    list->head.next->prev = ret;
    list->head.next = ret;
    ++list->length;
    return ret;
}

void kv_list_remove(kv_list_t * list, kv_t * kv) {
    assert(kv != &list->head);
    ref_obj_decref(kv->key);
    ref_obj_decref(kv->val);
    kv->prev->next = kv->next;
    kv->next->prev = kv->prev;
    free(kv);
    --list->length;
}

void kv_list_destory(kv_list_t * list) {
    while (list->length)
        kv_list_remove(list, list->head.next);
} 


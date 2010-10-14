#include <assert.h>
#include <string.h>
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

    c = (*p1) ^ (*p2);          // find the first bit diff
    while ((c & 128) == 0) {
        c <<= 1;
        ++i;
    }
    return i;
}

// 1: found, 0: not found, -1: error
static int _radix_tree_find(struct radix_tree_t * tree, 
        const char * key, 
        int create_if_not_exists,
        rt_node_t ** res) {
    // parent, child, new_node
    rt_node_t * p, *c, *n;
    unsigned int klen, bit_idx;
    
    // first find exist ones
    p = tree->root;
    c = p->left;
    klen = strlen(key);

    while (c->bit_idx > p->bit_idx) {
        p = c;
        c = _get_bit(key, klen, c->bit_idx) ? c->right : c->left;
    }

    // found
    if (strcmp(key, c->kv.key) == 0) {
        *res = c;
        return 1;
    }

    // not found
    if (!create_if_not_exists)
        return 0;

    // find a proper place to insert new node
    bit_idx = _diff_bit_idx(key, c->kv.key);
    p = tree->root;
    c = p->left;
    while (c->bit_idx > p->bit_idx && c->bit_idx < bit_idx) {
        p = c;
        c = _get_bit(key, klen, c->bit_idx) ? c->right : c->left;
    }

    // fill new node
    n = (rt_node_t *)kv_list_insert(tree->node_list, key, NULL);
    if (!n)
        return -1;
    n->bit_idx = bit_idx;
    if (_get_bit(key, klen, bit_idx)) {
        n->left = c;
        n->right = n;
    }
    else {
        n->left = n;
        n->right = c;
    }

    // link
    if (_get_bit(key, klen, p->bit_idx))
        p->right = n;
    else
        p->left = n;

    *res = n;
    return 1;
}

kv_list_t * kv_list_create(unsigned int elem_size) {
    assert (elem_size < sizeof(kv_t))
    kv_list_t * list = (kv_list_t *)malloc(sizeof(kv_list_t));
    if (!list)
        return NULL;
    kv_t * head = &list->head;
    head->next = head->prev = head;
    list->length = 0;
    list->elem_size = elem_size;
    return list;
}

kv_t * kv_list_insert(kv_list_t * list, const char * key, void * val) {
    kv_t * kv = (kv_t *)malloc(list->elem_size);
    if (!kv)
        return NULL;
    kv->key = key;
    kv->val = val;
    kv->next = list->head.next;
    kv->prev = &list->head;
    list->head.next->prev = kv;
    list->head.next = kv;
    ++list->length;
    return kv;
}

void kv_list_remove(kv_list_t * list, kv_t * kv) {
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
    radix_tree_t * tree = (radix_tree_t *)malloc(sizeof(radix_tree_t));
    if (!tree)
        goto RET_NULL;

    tree->node_list = kv_list_create(sizeof(rt_node_t));
    if (!tree->node_list)
        goto FREE_TREE;

    // pre insert an empty string key
    tree->root = (rt_node_t *)kv_list_insert(tree->node_list, "", NULL);
    tree->root->bit_idx = -1;
    tree->root->left = tree->root;
    tree->root->right = tree->root;
    return tree;

FREE_NODE_LIST:
    kv_list_destory(tree->node_list);
FREE_TREE:
    free(tree);
RET_NULL:
    return NULL;
}

void radix_tree_destory(radix_tree_t * tree) {
    kv_list_destory(tree->node_list);
    free(tree);
}

void * radix_tree_lookup(radix_tree_t * tree, const char * key) {
    rt_node_t * node;
    if (!_radix_tree_find(tree, key, 0, &node))
        return NULL;
    return node->kv.val;
}


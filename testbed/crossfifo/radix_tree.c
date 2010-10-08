#include <stdlib.h>
#include <string.h>
#include "radix_tree.h"

#define INIT_NODE(n,i,l,r,k,v) {(n)->bit_idx=i;\
    (n)->left=l;\
    (n)->right=r;\
    (n)->key=(k)?strdup(k):NULL;\
    (n)->val=v;}

typedef struct _link_node_t {
    rt_node_t node;
    struct _link_node_t * next;
    struct _link_node_t * prev;
} _link_node_t;

static rt_node_t * _malloc_node(struct radix_tree_t * tree) {
    _link_node_t * ret;
    if (!(ret = (_link_node_t *)malloc(sizeof(_link_node_t))))
        return NULL;

    // link list for all nodes of this tree
    _link_node_t * head = (_link_node_t *)tree->root;
    if (!head) {
        ret->next = ret->prev = ret;
        return (rt_node_t *)ret;
    }
    ret->next = head->next;
    ret->prev = head;
    head->next->prev = ret;
    head->next = ret;
    return (rt_node_t *)ret;
}

static void _free_node(rt_node_t * node) {
    // also need to free key's mem
    // since strdup (or NULL)
    free((void *)node->key);
    _link_node_t * n = (_link_node_t *)node;
    n->next->prev = n->prev;
    n->prev->next = n->next;
    free(n);
}

static void _free_all_nodes(struct radix_tree_t * tree) {
    _link_node_t * head = (_link_node_t *)tree->root;
    while (head->next != head)
        _free_node((rt_node_t *)head->next);
    _free_node((rt_node_t *)head);
}

void radix_tree_init(struct radix_tree_t * tree) {
    memset(tree, 0, sizeof(struct radix_tree_t));
    rt_node_t * root = tree->root = _malloc_node(tree);
    INIT_NODE(root, -1, root, root, NULL, NULL);
}

rt_node_t * radix_tree_insert(struct radix_tree_t * tree, 
        const char * key, 
        void * val) {
    return NULL;
}

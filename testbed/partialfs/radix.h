#ifndef _RADIX_H_
#define _RADIX_H_

#include <stddef.h>

/* radix tree */

typedef struct rdx_node_t {
    // leaf node content
    char * key;                 // owned
    void * val;                 // borrowed
    // inner node content
    int bitidx;
    struct rdx_node_t * left;       // 0 branch
    struct rdx_node_t * right;      // 1 branch
    struct rdx_node_t * parent;
} rdx_node_t;

typedef struct rdx_iter_t {
    rdx_node_t * root;
    rdx_node_t * inner;
    rdx_node_t * leaf;
} rdx_iter_t;

typedef struct rdx_tree_t {
    rdx_node_t root;
} rdx_tree_t;

/* this implementation doesn't support '\x00' in string
 * and can't insert empty string in it
 */

/* init/fini */

extern rdx_tree_t * rdx_tree_create(void);

extern void rdx_tree_destory(rdx_tree_t *);


/* manuplate functions */

extern rdx_node_t * rdx_tree_find(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int * err);

extern rdx_node_t * rdx_tree_ensure(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int * err);


/* iter functions, don't modify tree during iteration */

extern rdx_node_t * rdx_iter_begin(rdx_iter_t * iter, rdx_node_t * root);

extern rdx_node_t * rdx_iter_next(rdx_iter_t * iter);       // return NULL when end

#endif // _RADIX_H_

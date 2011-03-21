#ifndef _RADIX_H_
#define _RADIX_H_

#include <stddef.h>

/* radix tree
 * support only NULL-terminated string as key
 * support only find and insert operations (but no remove)
 */

typedef struct rdx_node_t {
    // leaf node content
    char * key;                     // owned
    size_t keylen;
    void * val;                     // borrowed
    // inner node content
    int bitidx;
    struct rdx_node_t * left;       // 0 branch
    struct rdx_node_t * right;      // 1 branch
    struct rdx_node_t * parent;
} rdx_node_t;

typedef struct rdx_tree_t {
    rdx_node_t root;
} rdx_tree_t;

typedef struct rdx_iter_t {
    rdx_node_t * root;
    rdx_node_t * inner;
    rdx_node_t * leaf;
} rdx_iter_t;

typedef struct rdx_prefix_iter_t {
    const char * key;
    size_t keylen;
    off_t checked;                      // key[:checked] is checked
    rdx_node_t * branch;                // not NULL: itering, NULL: end
    rdx_node_t * leaf;                  // not NULL: itering, NULL: not start
} rdx_prefix_iter_t;

/* manuplate functions */

extern void rdx_tree_init(rdx_tree_t *);

extern void rdx_tree_fini(rdx_tree_t *);

extern rdx_node_t * rdx_tree_lookup(rdx_tree_t * tree, const char * key, 
        int * err);

extern rdx_node_t * rdx_tree_ensure(rdx_tree_t * tree, const char * key, 
        int * err);

/* iter functions, don't modify tree during iteration */

// get all values in tree (in order)
extern rdx_node_t * rdx_iter_begin(rdx_tree_t * tree, 
        rdx_iter_t * iter);

extern rdx_node_t * rdx_iter_next(rdx_iter_t * iter);       // return NULL when end


// get all values that is prefix of a key (from shortest to longest)
extern rdx_node_t * rdx_prefix_iter_begin(rdx_tree_t * tree, const char * key,
        rdx_prefix_iter_t * iter);

extern rdx_node_t * rdx_prefix_iter_next(rdx_prefix_iter_t * iter);


#endif // _RADIX_H_

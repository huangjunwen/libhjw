#ifndef _RADIX_H_
#define _RADIX_H_

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

extern rdx_tree_t * rdx_tree_create(void);

extern void rdx_tree_destory(rdx_tree_t *);


/* iter functions, don't modify tree during iteration */

extern rdx_node_t * rdx_iter_begin(rdx_iter_t * iter, rdx_node_t * root);

extern rdx_node_t * rdx_iter_next(rdx_iter_t * iter);


#endif // _RADIX_H_

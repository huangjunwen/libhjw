#ifndef _RADIX_H_
#define _RADIX_H_

#include <stdlib.h>

/* radix tree */

typedef struct rdx_node_t {
    char * key;                     // owned
    void * val;                     // borrowed
    int bitidx;
    struct rdx_node_t * left;       // 0 branch
    struct rdx_node_t * right;      // 1 branch
    struct rdx_node_t * prev;
    struct rdx_node_t * next;       // link all in tree's all field
} rdx_node_t;

typedef struct rdx_tree_t {
    rdx_node_t root;
    rdx_node_t * all;    
} rdx_tree_t;


unsigned char _get_bit(const char * s, size_t bitlen, int bitidx);

int _diff_bitidx(const char * s1, const char * s2);

#endif // _RADIX_H_

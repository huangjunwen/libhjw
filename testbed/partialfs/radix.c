#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "radix.h"

#define IS_INNER(node, parent) ((node)->bitidx > (parent)->bitidx)

static char * EMPTY_STR = "";

#define _2x(c) (c), (c)
#define _4x(c) _2x(c), _2x(c)
#define _8x(c) _4x(c), _4x(c)
#define _16x(c) _8x(c), _8x(c)
#define _32x(c) _16x(c), _16x(c)
#define _64x(c) _32x(c), _32x(c)
#define _128x(c) _64x(c), _64x(c)

// examples:
//
// HIGHEST_DIFF_BIT[35]   = HIGHEST_DIFF_BIT[00100011] = 2
//                                         ^
//
// HIGHEST_DIFF_BIT[129]  = HIGHEST_DIFF_BIT[10000001] = 0
//                                       ^
//
static int HIGHEST_DIFF_BIT[256] = {/*impossible*/-1, 7, 
    _2x(6), 
    _4x(5),
    _8x(4),
    _16x(3),
    _32x(2),
    _64x(1),
    _128x(0)
};

static inline unsigned char _get_bit(const char * s, size_t bitlen, int bitidx) {
    return (bitidx < 0 || bitidx >= bitlen) ? 0 :
        (s[bitidx >> 3] & (0x80 >> (bitidx & 0x7)));
}

// s1 and s2 must not be the same
static inline int _diff_bitidx(const char * s1, const char * s2) {
    const char * p1, * p2;
    int r;

    p1 = s1;
    p2 = s2;
    r = 0;
    while (*p1 == *p2) {
        ++r;
        ++p1;
        ++p2;
    }
    r <<= 3;                // *= 8

    return r + HIGHEST_DIFF_BIT[(*p1) ^ (*p2)];
}

rdx_tree_t * rdx_tree_create() {
    rdx_tree_t * ret;
    rdx_node_t * root;
    if ((ret = (rdx_tree_t *)malloc(sizeof(rdx_tree_t))) == NULL)
        return NULL;
    root = &ret->root;
    root->key = EMPTY_STR;
    root->val = NULL;
    root->bitidx = -1;
    root->left = root;
    root->right = root->parent = NULL;
    return ret;
}

void rdx_tree_destory(rdx_tree_t * tree) {
    rdx_iter_t iter;
    rdx_node_t * node, * root, * all;

    // link them all using 'val' field
    all = NULL;
    root = &tree->root;
    node = rdx_iter_begin(&iter, root);
    while (node) {
        // don't free root
        if (node == root)
            continue;
        node->val = all;
        all = node;
        node = rdx_iter_next(&iter);
    }

    // free
    while (all) {
        node = all;
        all = (rdx_node_t *)all->val;
        free(node->key);
        free(node);
    }

    free(tree);
}
    
rdx_node_t * rdx_iter_begin(rdx_iter_t * iter, rdx_node_t * root) {
    rdx_node_t * p, * c;
    int bitidx;
    
    bitidx = root->bitidx;
    p = root;
    c = p->left;
    
    // find the left most leaf
    while (IS_INNER(c, p)) {
        p = c;
        c = p->left;
    }

    iter->root = root;
    iter->inner = p;
    iter->leaf = c;

    // "" is not part of the tree
    if (iter->leaf->key == EMPTY_STR)
        return rdx_iter_next(iter);
    return iter->leaf;
}

rdx_node_t * rdx_iter_next(rdx_iter_t * iter) {
    rdx_node_t * p, * c, * r;
    int bitidx;

    // already end
    if (iter->root == NULL)
        return NULL;

    r = iter->root;
    p = iter->inner;
    c = iter->leaf;

    // find the first ancestor (include iter->inner itself)
    // whose right child has not yet traverse
    while (p->right == c) {
        if (p == r)
            goto ITER_END;
        c = p;
        p = p->parent;
    }

    // !!special case: we can't traverse a tree's 
    //      root's right child
    if (p->parent == NULL) {
        assert(p == r);
        goto ITER_END;
    }

    // find the left most leaf
    c = p->right;
    while (IS_INNER(c, p)) {
        p = c;
        c = p->left;
    }

    iter->inner = p;
    iter->leaf = c;
    return c;

ITER_END:
    memset(iter, 0, sizeof(rdx_iter_t));
    return NULL;
}

// find a leaf node according to a key
// note that the returned key is not necessary the same
// as the the argument
static inline rdx_node_t * _rdx_lookup_leaf(rdx_tree_t * tree, 
        const char * key,
        size_t keylen) {

    // parent, child
    rdx_node_t * p, * c;
    size_t keybitlen;
    int bitidx;

    // empty string is reserved
    if (!key[0])
        return NULL;

    if (!keylen)
        keylen = strlen(key);
    keybitlen = (keylen << 3);
    p = &tree->root;
    c = p->left;

    // iter to the one of the leaf
    while (IS_INNER(c, p)) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    return c;
}

rdx_node_t * rdx_tree_find(rdx_tree_t * tree, const char * key, 
        size_t keylen, 
        int * err) {
    rdx_node_t * leaf;

    *err = 1;
    leaf = _rdx_lookup_leaf(tree, key, keylen);
    if (!leaf)
        return NULL;
    
    *err = 0;
    // found
    if (strcmp(key, leaf->key) == 0)
        return leaf;
    return NULL;

}

rdx_node_t * rdx_tree_ensure(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int * err) {
    rdx_node_t * p, * c, * n, * leaf;
    size_t keybitlen;
    int bitidx;

    *err = 1;
    leaf = _rdx_lookup_leaf(tree, key, keylen);
    if (!leaf)
        return NULL;
    
    // found
    if (strcmp(key, leaf->key) == 0) {
        *err = 0;
        return leaf;
    }

    // not found
    // find proper place to insert new node
    // the new node is to be insert between p and c
    bitidx = _diff_bitidx(key, leaf->key);
    p = &tree->root;
    c = p->left;
    while (IS_INNER(c, p) && bitidx > c->bitidx) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    // fill new node
    n = (rdx_node_t *)malloc(sizeof(rdx_node_t));
    if (n == NULL)
        return NULL;
    if ((n->key = strndup(key, keylen)) == NULL) {
        free(n);
        return NULL;
    }
    n->val = NULL;
    n->bitidx = bitidx;
    if (_get_bit(key, keybitlen, bitidx)) {
        n->left = c;
        n->right = n;
    }
    else {
        n->left = n;
        n->right = c;
    }
    n->parent = p;

    // link in tree
    if (IS_INNER(c, p))
        c->parent = n;

    if (p->left == c)
        p->left = n;
    else
        p->right = n;

    *err = 0;
    return n;
}



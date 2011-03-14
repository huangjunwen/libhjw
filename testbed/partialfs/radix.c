#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "radix.h"

#define IS_INNER(node, parent) ((node)->bitidx > (parent)->bitidx)
#define IS_LEAF(node, parent) ((node)->bitidx <= (parent)->bitidx)

static char * EMPTY_STR = "";

// _get_bit("ab", 0..7) -> 10000110 (97)
unsigned char _get_bit(const char * s, size_t bitlen, int bitidx) {
    return (bitidx < 0 || bitidx >= bitlen) ? 0 :
        (s[bitidx >> 3] & (1 << (bitidx & 15)));
}

int _diff_bitidx(const char * s1, const char * s2) {
    // ref:
    //  http://www.matrix67.com/blog/archives/3985
    //  http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
    //  http://en.wikipedia.org/wiki/De_Bruijn_sequence
    // we need 8-bits seq, how to get it:
    // B(2, 3): 00010111   (0x17)
    //   000, 001, 010, 101, 011, 111, 110, 100
    //   0, 1, 2, 5, 3, 7, 6, 4
    //   m[0] = 0, m[1] = 1, m[2] = 2, m[5] = 3,
    //   m[3] = 4, m[7] = 5, m[6] = 6, m[4] = 7
    static const int _DeBruijinBitPos[8] = {
        0, 1, 2, 4, 7, 3, 6, 5
    };
    const char * p1, * p2;
    int8_t c;
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

    c = (*p1) ^ (*p2);      // xor, so the first diff bit will the
                            // first 1 from the low end
    return r + _DeBruijinBitPos[((uint8_t)((c & -c) * 0x17)) >> 5];
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

// return 
//  -1: error
//  0: not found
//  1: found
//  2: insert (create_if_not_exists)
static int _rdx_tree_lookup(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int create_if_not_exists,
        rdx_node_t ** res) {

    // parent, child, new_node, point_to_child
    rdx_node_t * p, * c, * n, ** pc;
    size_t keybitlen;
    int bitidx;

    if (!keylen)
        keylen = strlen(key);
    keybitlen = (keylen << 3);
    p = &tree->root;
    c = p->left;

    // firstly find exist one
    while (IS_INNER(c, p)) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    // now p is the inner node, c is the leaf node
    if (strcmp(key, c->key) == 0) {
        *res = c;
        return 1;
    }

    // doesn't need to insert
    if (!create_if_not_exists)
        return 0;

    // find proper place to insert new node
    bitidx = _diff_bitidx(key, c->key);
    p = &(tree->root);
    pc = &(p->left);
    c = *pc;
    while (IS_INNER(c, p) && c->bitidx < bitidx) {
        p = c;
        pc = _get_bit(key, keybitlen, c->bitidx) ? &(c->right) : &(c->left);
        c = *pc;
    }

    // fill new node
    n = (rdx_node_t *)malloc(sizeof(rdx_node_t));
    if (n == NULL)
        return -1;
    if ((n->key = strndup(key, keylen)) == NULL) {
        free(n);
        return -1;
    }
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
    *pc = n;

    *res = n;
    return 2;
}

rdx_node_t * rdx_tree_find(rdx_tree_t * tree, const char * key, 
        size_t keylen, 
        int * err) {
    rdx_node_t * node;

    if (!key[0]) {
        *err = -1;
        return NULL;
    }

    *err = _rdx_tree_lookup(tree, key, keylen, 0, &node);
    if (*err <= 0) 
        return NULL;
    return node;
}

rdx_node_t * rdx_tree_ensure(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int * err) {
    rdx_node_t * node;

    if (!key[0]) {
        *err = -1;
        return NULL;
    }

    *err = _rdx_tree_lookup(tree, key, keylen, 1, &node);
    if (*err <= 0)
        return NULL;
    return node;
}

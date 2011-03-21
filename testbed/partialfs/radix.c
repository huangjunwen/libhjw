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

void rdx_tree_init(rdx_tree_t * tree) {
    rdx_node_t * root;
    root = &tree->root;
    root->key = EMPTY_STR;
    root->keylen = 0;
    root->val = NULL;
    root->bitidx = -1;
    root->left = root;
    root->right = root->parent = NULL;
}

void rdx_tree_fini(rdx_tree_t * tree) {
    rdx_iter_t iter;
    rdx_node_t * node, * root, * all;

    // link them all using 'val' field
    all = NULL;
    root = &tree->root;
    node = rdx_iter_begin(tree, &iter);
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
}
    
rdx_node_t * rdx_iter_begin(rdx_tree_t * tree, 
        rdx_iter_t * iter) {

    rdx_node_t * p, * c;

    iter->root = p = &tree->root;
    c = p->left;
    
    // find the left most leaf
    while (IS_INNER(c, p)) {
        p = c;
        c = p->left;
    }

    iter->inner = p;
    iter->leaf = c;

    // empty str is preserved
    if (!iter->leaf->key[0])
        return rdx_iter_next(iter);
    return iter->leaf;
}

rdx_node_t * rdx_iter_next(rdx_iter_t * iter) {

    rdx_node_t * p, * c, * r;

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
    iter->root = NULL;
    return NULL;
}

rdx_node_t * rdx_prefix_iter_begin(rdx_tree_t * tree, const char * key,
        rdx_prefix_iter_t * iter) {

    if (!key[0])
        return NULL;

    iter->key = key;
    iter->keylen = strlen(key);
    iter->checked = 0;
    iter->branch = &tree->root;
    iter->leaf = NULL;

    return rdx_prefix_iter_next(iter);
}

rdx_node_t * rdx_prefix_iter_next(rdx_prefix_iter_t * iter) {
    rdx_node_t * p, * c;
    const char * key;
    size_t keylen;
    size_t keybitlen;
    off_t checked;

    // already end;
    if (iter->branch == NULL)
        return NULL;

    p = iter->branch;
    if (iter->leaf == NULL)
        c = p->left;            // not start yet
    else
        c = p->right;

    key = iter->key;
    keylen = iter->keylen;
    keybitlen = (keylen << 3);

    while (IS_INNER(c, p)) {
        p = c;

        // left is ignore
        if (!_get_bit(key, keybitlen, c->bitidx)) {
            c = p->left;
            continue;
        }

        // the key ready to go right

        // save state
        // and go to the left most of the sibling
        iter->branch = p;
        c = p->left;
        while (IS_INNER(c, p)) {
            p = c;
            c = p->left;
        }

        // c's key is a prefix of key only when:
        //      c->key is not empty str
        //      c->key == key[:c->keylen] which implies:
        //          c->key[branch->bitidx >> 3] == '\0'
        if (c->keylen && c->keylen == (iter->branch->bitidx >> 3)) {
            // cmp
            checked = iter->checked;
            if (strncmp(c->key + checked, key + checked, 
                        c->keylen - checked) == 0) {
                // it's prefix
                iter->checked = c->keylen;
                iter->leaf = c;
                return c;
            }
            // since c->key[:(branch->bitidx>>3)] is the common
            // prefix of the sub tree (root of branch)
            // so no need to iter further
            iter->branch = NULL;
            return NULL;
        }

        // false alarm, continue
        p = iter->branch;
        c = p->right;
    }

    // last iter
    iter->branch = NULL;

    // finally check the leaf node
    checked = iter->checked;
    if (strncmp(c->key + checked, key + checked, 
            c->keylen - checked))
        return NULL;

    iter->checked = checked;
    iter->leaf = c;
    return c;

}

// find a leaf node according to a key
// note that the returned key is not necessary the same
// as the the argument
static inline rdx_node_t * _rdx_lookup_leaf(rdx_tree_t * tree, 
        const char * key,
        size_t keybitlen) {

    // parent, child
    rdx_node_t * p, * c;

    p = &tree->root;
    c = p->left;

    // iter to the one of the leaf
    while (IS_INNER(c, p)) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    return c;
}

rdx_node_t * rdx_tree_lookup(rdx_tree_t * tree, const char * key, 
        int * err) {

    size_t keylen;
    rdx_node_t * leaf;

    *err = 1;
    keylen = strlen(key);

    // empty str is preserved
    if (keylen == 0)
        return NULL;

    leaf = _rdx_lookup_leaf(tree, key, keylen << 3);
    
    *err = 0;

    // found
    if (strcmp(key, leaf->key) == 0)
        return leaf;
    return NULL;

}

rdx_node_t * rdx_tree_ensure(rdx_tree_t * tree, const char * key, 
        int * err) {
    rdx_node_t * p, * c, * n, * leaf;
    size_t keylen, keybitlen;
    int bitidx;

    *err = 1;
    keylen = strlen(key);

    // empty str is preserved
    if (keylen == 0)
        return NULL;

    keybitlen = (keylen << 3);
    leaf = _rdx_lookup_leaf(tree, key, keybitlen);
    
    // found
    if (strcmp(key, leaf->key) == 0) {
        *err = 0;
        return leaf;
    }

    // not found
    // find proper place to insert new node
    bitidx = _diff_bitidx(key, leaf->key);
    p = &tree->root;
    c = p->left;
    while (IS_INNER(c, p) && bitidx > c->bitidx) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    // the new node will be inserted as a child of p
    // and the parent of c
    // fill new node
    n = (rdx_node_t *)malloc(sizeof(rdx_node_t));
    if (n == NULL)
        return NULL;
    if ((n->key = strndup(key, keylen)) == NULL) {
        free(n);
        return NULL;
    }
    n->keylen = keylen;
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


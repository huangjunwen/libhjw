#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "radix.h"

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
    root->key = "";
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
    
void rdx_iter_begin(rdx_iter_t * iter, rdx_node_t * root) {
    rdx_iter_t * p, * c;
    int bitidx;
    
    bitidx = root->bitidx;
    p = root;
    c = p->left;
    
    // find the left most
    while (c->bitidx > p->bitidx) {
        p = c;
        c = p->left;
    }

    iter->root = root;
    iter->inner = p;
    iter->leaf = c;

    return iter->leaf;
}

rdx_node_t * rdx_iter_next(rdx_iter_t * iter) {
    rdx_iter_t * p, * c, * r;
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

    // find the left most
    c = p->right;
    while (c->bitidx > p->bitidx) {
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

rdx_node_t * _search_leaf(const char * key, size_t keybitlen,
        rdx_node_t * root, rdx_node_t ** inner) {

    rdx_node_t * p, * c;

    c = root;
    do {
        p = c;
        c = _get_bit(key, keybitlen, p->bitidx) ? c->right : c->left;
    }
    while (c->bitidx > p->bitidx);

    if (inner)
        *inner = p;
    return c;
}

rdx_node_t * rdx_tree_find(rdx_node_t * tree, const char * key) {

    rdx_node_t * node;
    size_t keybitlen;

    keybitlen = (strlen(key) << 3);
    node = _search_leaf(key, keybitlen, &tree->root, 0);

    if (strcmp(key, node->key) == 0)
        return node;
    return NULL;
}

rdx_node_t * rdx_tree_insert(rdx_tree_t * tree, const char * key, void * val) {

}

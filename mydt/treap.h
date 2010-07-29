// vim:fdm=marker:nu:nowrap

#ifndef _TREAP_H_
#define _TREAP_H_

#include "typedefs.h"
#include "mem_pool.h"
#include "mydt_config.h"

typedef struct treapNode treapNode;

struct treapNode {
    void * val;
    uint32_t priority;              // the greater the closer to the root
    treapNode * parent;             // if parent == 0, this node is root
    treapNode * left;
    treapNode * right;
};

typedef struct {
    // if node == 0, this is an 'empty' node, we can insert new node in it
    treapNode * node;
    treapNode * parent;
    treapNode ** pnode;
} treapIter;

typedef struct {
    treapNode * root;
    memPool pool;
} treap;

void treap_reset(treap * t);
boolean_t treap_init(treap * t);
void treap_finalize(treap * t);
void * treap_insert_at(treap * t, treapIter * iter, void * val);
void treap_delete(treap * t, void * ptr);

/*********************************
 * interface
 *********************************/

typedef treap BST;
typedef treapIter BSTIter;

#define BST_INIT treap_init
#define BST_FINALIZE treap_finalize
#define BST_RESET treap_reset
#define BST_INSERT_AT treap_insert_at
#define BST_DELETE treap_delete

#define BST_ITER_INIT(bst, piter) (piter)->node = (bst)->root; \
                                 (piter)->parent = 0; \
                                 (piter)->pnode = &((bst)->root)

#define BST_ITER_NOTNIL(piter) ((piter)->node != 0)
#define BST_ITER_DEREF(piter) ((piter)->node->val)
#define BST_ITER_FORWARD(piter) (piter)->parent = (piter)->node; \
                                (piter)->pnode = &((piter)->node->right); \
                                (piter)->node = *((piter)->pnode)

#define BST_ITER_BACKWARD(piter) (piter)->parent = (piter)->node; \
                                (piter)->pnode = &((piter)->node->left); \
                                (piter)->node = *((piter)->pnode)


#endif

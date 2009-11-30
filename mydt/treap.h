// vim:fdm=marker:nu:nowrap:encoding=gbk

#include "config.h"
#include "types.h"
#include "mem_pool.h"

typedef struct treapNode treapNode;

struct treapNode {
	void * val;
	uint32 priority;				// the greater the closer to the root
	treapNode * parent;				// 为空时表示为 root
	treapNode * left;
	treapNode * right;
};

typedef struct {
	treapNode * node;
	// 当 node 为空时, 表示这是一个空节点(不存在的), 此时 parent 以及 pnode
	// 联合指示这个空节点的位置
	// 若 parent 也是空的, 则表示 treap 也是空的
	treapNode * parent;
	treapNode ** pnode;
} treapIter;

typedef struct {
	treapNode * root;
	memPool pool;
} treap;

void treap_reset(treap * t);
boolean treap_init(treap * t);
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


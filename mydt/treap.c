// vim:fdm=marker:nu:nowrap

#include <assert.h>
#include <stdlib.h>
#include "treap.h"

void treap_reset(treap * t) {
	mem_pool_reset(&t->pool);
	t->root = 0;
}

boolean treap_init(treap * t) {
	if (!mem_pool_init(&t->pool, sizeof(treapNode), 128))
		return 0;
	treap_reset(t);
	return 1;
}

void treap_finalize(treap * t) {
	mem_pool_finalize(&t->pool);
}

INTERNAL void _rotate(treap * t, treapNode * p, treapNode * c) {
	// g for grantparent
	treapNode * g = p->parent;
	// pp for pointer to parent
	treapNode ** pp = g ? (p == g->left ? (&g->left) : (&g->right)) : (&t->root);
	if (c == p->left) {			// rotate right
		p->left = c->right;
		if (c->right) { c->right->parent = p; }
		c->right = p;
		p->parent = c;
	} else {					// rotate left
		p->right = c->left;
		if (c->left) { c->left->parent = p; }
		c->left = p;
		p->parent = c;
	}
	c->parent = g;
	*pp = c;
}

void * treap_insert_at(treap * t, treapIter * iter, void * val) {
	// assert(!iter->node);
	treapNode * n = (treapNode *)mem_pool_get(&t->pool);
	n->val = val;
	n->priority = rand();
	n->parent = iter->parent;
	n->left = n->right = 0;
	*(iter->pnode) = n;

	treapNode * p;
	// while n is not root and it's parent's priority is less than it (reverse)
	while ((p = n->parent) && p->priority < n->priority) {
		_rotate(t, p, n);
	}
	return (void *)n;
}

void treap_delete(treap * t, void * ptr) {
	if (!ptr)
		return;
	treapNode * n = (treapNode *)ptr;
	treapNode * c;
	while (1) {
		c = n->left;
		if (!c || (n->right && n->right->priority > c->priority))
			c = n->right;
		if (!c)
			break;
		_rotate(t, n, c);
	}
	// alter the leaf node
	treapNode * p = n->parent;
	if (p) {
		if (p->left == n)
			p->left = 0;
		else
			p->right = 0;
	} else {
		t->root = 0;
	}
	mem_pool_release(&t->pool, n);
}


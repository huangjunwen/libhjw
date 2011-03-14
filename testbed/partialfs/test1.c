#include <stdio.h>
#include <stdlib.h>
#include "extend_printf.h"
#include "radix.h"

int main() {
    extend_printf();
    char * s1;
    size_t l1, sl1;
    int err;
    rdx_tree_t * tree;
    rdx_node_t * node, * root;
    rdx_iter_t iter;

    if (!(tree = rdx_tree_create())) {
        printf("can't create tree\n");
        return 1;
    }

    root = &tree->root;
    printf("root idx: %p, %d, %s\n", root, root->bitidx, root->key);
    l1 = 0;
    while (1) {
        s1 = NULL;
        if ((sl1 = getline(&s1, &l1, stdin)) < 0)
            break;
        s1[--sl1] = '\0';
        if (!sl1) {
            free(s1);
            break;
        }
        node = rdx_tree_ensure(tree, s1, sl1, &err);
        if (!node)
            printf("can't insert %*s \n", sl1, s1);
        else
            /*
            printf("inserted %*s at %p, %d:\n"
                    "\tparent %p, %d, %s\n"
                    "\tleft   %p, %d, %s\n"
                    "\tright  %p, %d, %s\n", 
                    sl1, s1, node, node->bitidx, 
                    node->parent, node->parent->bitidx, node->parent->key,
                    node->left, node->left->bitidx, node->left->key,
                    node->right, node->right->bitidx, node->right->key);
                    */
            check_tree(tree, 1);
        free(s1);
    }

    node = rdx_iter_begin(&iter, &tree->root);
    while (node) {
        printf("%s\n", node->key);
        node = rdx_iter_next(&iter);
    }

    rdx_tree_destory(tree);

    return 0;
}

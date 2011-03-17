#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../radix.h"

#define IS_INNER(node, parent) ((node)->bitidx > (parent)->bitidx)

static char * bytes_repr(const char * s) {
    const char * ps;
    char * buf, * pbuf;
    char c;
    size_t len, buflen;

    len = strlen(s) + 1;
    buflen = len * 10 + 1;
    buf = (char *)malloc(buflen);
    if (!buf)
        return NULL;

    buf[0] = '<';
    ps = s;
    pbuf = buf + 1;
    while (len--) {
        if (pbuf != buf + 1) {
            *pbuf++ = ',';
            *pbuf++ = ' ';
        }
        c = *ps++;
        *pbuf++ = (c >> 7) ? '1' : '0';
        *pbuf++ = (c & 64) ? '1' : '0';
        *pbuf++ = (c & 32) ? '1' : '0';
        *pbuf++ = (c & 16) ? '1' : '0';
        *pbuf++ = (c & 8) ? '1' : '0';
        *pbuf++ = (c & 4) ? '1' : '0';
        *pbuf++ = (c & 2) ? '1' : '0';
        *pbuf++ = (c & 1) ? '1' : '0';
    }
    buf[buflen - 2] = '>';
    buf[buflen - 1] = '\0';
    return buf;
}

static void print_node(const char * titile, rdx_node_t * node) {
    char * bytes = bytes_repr(node->key);
    if (!bytes)
        bytes = "";
    printf("%s: %s %s, %d at %p\n", titile, bytes, node->key, node->bitidx, node);
    if (bytes[0])
        free(bytes);
}

static int check_node_err(rdx_node_t * node, int islastchild, char * ident) {
    char * childident, * tmp;
    int i;
    int ret;
    ret = 1;
    asprintf(&childident, "%s%c   ", ident, islastchild ? ' ' : '|');

    // check pointers
    if (node->left && IS_INNER(node->left, node) && node->left->parent != node) {
        printf("node %p 's left child is %p, but its parent is %p\n",
            node, node->left, node->left->parent);
        goto END;
    }
    if (node->right && IS_INNER(node->right, node) && node->right->parent != node) {
        printf("node %p 's right child is %p, but its parent is %p\n",
            node, node->right, node->right->parent);
        goto END;
    }

    // itself
    printf("%s%c-- %d (%p)\n", ident, islastchild ? '`' : '|', node->bitidx, node);


    // check left
    if (IS_INNER(node->left, node)) {
        if (check_node_err(node->left, node->right ? 0 : 1, childident)) 
            goto END;
    }
    else {
        tmp = bytes_repr(node->left->key);
        printf("%s%c-- %s %s\n", childident, node->right ? '|' : '`', node->left->key, tmp);
        free(tmp);
    }

    // check right
    if (node->right) {
        if (IS_INNER(node->right, node)) {
            if (check_node_err(node->right, 1, childident)) 
                goto END;
        }
        else {
            tmp = bytes_repr(node->right->key);
            printf("%s`-- %s %s\n", childident, node->right->key, tmp);
            free(tmp);
        }
    }

    ret = 0;
END:
    free(childident);
    return ret;
}

int check_tree_err(rdx_tree_t * tree) {
    return check_node_err((&tree->root)->left, 1, "");
}


int main() {
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
    print_node("root    ", root);
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
            printf("can't insert %s\n", s1);
        else {
#if 0
            print_node(    "inserted", node);
            if (node->parent);
                print_node("parent  ", node->parent);
            if (node->left);
                print_node("left    ", node->left);
            if (node->right);
                print_node("right   ", node->right);
#endif
            err = check_tree_err(tree);
            printf("check tree %s\n", err ? "failed" : "ok");
        }

        free(s1);
        if (err)
            break;
    }

    node = rdx_iter_begin(&iter, &tree->root);
    while (node) {
        printf("%s\n", node->key);
        node = rdx_iter_next(&iter);
    }

    rdx_tree_destory(tree);

    return 0;
}

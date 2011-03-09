//#include <fuse.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * A readonly partial view of the orginal system
 */

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
    rdx_node_t * root;
    rdx_node_t * all;    
} rdx_tree_t;

// GET_BIT("ab", 0..7) -> 10000110 (97)
inline unsigned char _get_bit(const char * s, size_t bitlen, int bitidx) {
    return (bitidx < 0 || bitidx >= bitlen) ? 0 :
        (s[bitidx >> 3] & (1 << (bitidx & 15)));
}

inline int _diff_bitidx(const char * s1, const char * s2) {
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
    if ((ret->root = (rdx_node_t *)malloc(sizeof(rdx_node_t))) == NULL)
        goto FREE_TREE;
    root = ret->root;
    if ((root->key = strdup("")) == NULL)
        goto FREE_ROOT;
    root->val = NULL;
    root->bitidx = -1;
    root->left = root;
    root->right = NULL;
    root->prev = root;
    root->next = root;
    ret->all = root;
    return ret;
FREE_ROOT:
    free(ret->root);
FREE_TREE:
    free(ret);
    return NULL;
}

void rdx_tree_destroy(rdx_tree_t * tree) {
    rdx_node_t * node, * end;
    node = end = tree->all;
    do {
        free(node->key);
        free(node);
        node = node->next;
    } while (node != end);
    free(tree);
}

// return 
//  -1: error
//  0: not found
//  1: found
//  2: insert (val passed in is not NULL)
int rdx_tree_find_or_insert(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        int create,
        rdx_node_t ** res) {

    // parent, child, new_node, point_to_child
    rdx_node_t * p, * c, * n, ** pc;
    size_t keybitlen;
    int bitidx;

    if (!keylen)
        keylen = strlen(key);
    keybitlen = (keylen << 3);
    p = tree->root;
    c = p->left;

    // first find exist ones
    while (c->bitidx > p->bitidx) {
        p = c;
        c = _get_bit(key, keybitlen, c->bitidx) ? c->right : c->left;
    }

    // now c is the leaf node and found
    if (strcmp(key, c->key) == 0) {
        *res = c;
        return 1;
    }

    // doesn't need to insert
    if (!create)
        return 0;

    // find proper place to insert new node
    bitidx = _diff_bitidx(key, c->key);
    p = tree->root;
    pc = &(p->left);
    c = *pc;
    while (c->bitidx > p->bitidx && c->bitidx < bitidx) {
        p = c;
        pc = _get_bit(key, keybitlen, c->bitidx) ? &(c->right) : &(c->left);
        c = *pc;
    }

    // fill new node and link in tree
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

    *pc = n;

    // link in list
    n->next = tree->all;
    n->prev = tree->all->prev;
    tree->all->prev->next = n;
    tree->all->prev = n;

    *res = n;
    return 1;
}

/*
int rdx_tree_delete(rdx_tree_t * tree, const char * key, 
        size_t keylen,
        void ** val) {
    // grand_parent, parent, child, point_to_parent
    rdx_node_t * g, * p, * c, ** pp;
    size_t keybitlen;
    int bitidx;
    
    if (!keylen)
        keylen = strlen(key);
    keybitlen = (keylen << 3);
    g = tree->root;
    pp = &(g->left);
    p = *pp;

}
*/

#if 0

int partial_getattr(const char * path, struct stat * stbuf) {
}

int partial_readlink(const char * path, char * buf, size_t sz) {
}

int partial_mknod(const char * path, mode_t mode, dev_t dev) {
}

int partial_mkdir(const char * path, mode_t mode) {
}

int partial_unlink(const char * path) {
}

int partial_rmdir(const char * path) {
}

int partial_symlink(const char * path, const char * link) {
}

int partial_rename(const char * path, const char * newpath) {
}

int partial_link(const char * path, const char * link) {
}

int partial_chmod(const char * path, mode_t mode) {
}

int partial_chown(const char * path, uid_t uid, gid_t gid) {
}

int partial_truncate(const char * path, off_t off) {
}

int partial_open(const char * path, struct fuse_file_info * fi) {
}

int partial_read(const char * path, char * buf, size_t size, off_t off,
         struct fuse_file_info * fi) {
}
int partial_write(const char * path, const char * buf, size_t size, off_t off,
          struct fuse_file_info * fi) {
}
int partial_statfs(const char * path, struct statvfs * stat) {
}

int partial_flush(const char * path, struct fuse_file_info * fi) {
}

int partial_release(const char * path, struct fuse_file_info * fi) {
}

int partial_fsync(const char * path, int datasync, struct fuse_file_info * fi) {
}

int partial_setxattr(const char * path, const char * name, const char * value, 
        size_t size, int flags) {
}

int partial_getxattr(const char * path, const char * name, char * value, size_t size) {
}

int partial_listxattr(const char * path, char * list, size_t size) {
}

int partial_removexattr(const char * path, const char * name) {
}

int partial_opendir(const char * path, struct fuse_file_info * fi) {
}

int partial_readdir(const char * path, void * buf, fuse_fill_dir_t filler, 
        off_t off,
        struct fuse_file_info * fi) {
}

int partial_releasedir(const char * path, struct fuse_file_info * fi) {
}

int partial_fsyncdir(const char * path, int datasync, struct fuse_file_info * fi) {
}

void *partial_init(struct fuse_conn_info * conn) {
}

void partial_destroy(void * userdata) {
}

int partial_access(const char * path, int mask) {
}

int partial_create(const char * path, mode_t mode, struct fuse_file_info * fi) {
}

int partial_ftruncate(const char * path, off_t off, struct fuse_file_info * fi) {
}

int partial_fgetattr(const char * path, struct stat * statbuf, struct fuse_file_info * fi) {
}

int partial_lock(const char * path, struct fuse_file_info * fi, int cmd,
         struct flock * fl) {
}

int partial_utimens(const char * path, const struct timespec tv[2]) {
}

struct fuse_operations partialfs_oper;

#endif

int main() {
    int ret;
    rdx_node_t * node;
    rdx_tree_t * tree = rdx_tree_create();
    if (!tree) {
        printf("Bad tree\n");
        return 1;
    }
    
    ret = rdx_tree_find_or_insert(tree, "hello", 0, 1, &node);
    if (ret != 2)
        goto err;
    node->val = (void *)1;

    ret = rdx_tree_find_or_insert(tree, "world", 0, 1, &node);
    if (ret != 2)
        goto err;
    node->val = (void *)2;

    ret = rdx_tree_find_or_insert(tree, "he", 0, 1, &node);
    if (ret != 2)
        goto err;
    node->val = (void *)3;

    ret = rdx_tree_find_or_insert(tree, "abc", 0, 1, &node);
    if (ret != 2)
        goto err;
    node->val = (void *)4;

    ret = rdx_tree_find_or_insert(tree, "he", 0, 0, &node);
    if (ret != 1)
        goto err;
    printf("Found he: %d\n", (int)node->val);
err:
    printf("bad insert %d\n", ret);
    rdx_tree_destroy(tree);
    return 1;
}

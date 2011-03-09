#include <fuse.h>
#include <stdlib.h>
#include <string.h>

/*
 * A readonly partial view of the orginal system
 */

/* radix tree */

typedef struct rd_node_t {
    const char * key;
    void * val;
    int idx;
    struct rd_node_t * left;        // 0
    struct rd_node_t * right;       // 1
    struct rd_node_t * next;        // link all in tree's all field
} rd_node_t;

typedef struct rd_tree_t {
    rd_node_t * root;
    rd_node_t * all;    
} rd_tree_t;

// GET_BIT("ab", 0..7) -> 10000110 (97)
#define GET_BIT(s, i) s[i >> 3] & (1 << (i & 15));

rd_tree_t * rd_tree_create() {
    rd_tree_t * ret;
    ret = (rd_tree_t *)malloc(sizeof(rd_tree_t));
    ret->root = ret->all = NULL;
    return ret;
}

void rd_tree_destroy(rd_tree_t * tree) {
    rd_node_t * node, * next;
    next = tree->all;
    while (node = next) {
        next = node->next;
        free(node);
    }
    free(tree);
}

rd_node_t * rd_tree_lookup(rd_tree_t * tree, const char * key) {
    rd_node_t * root, * node;
    size_t keylen;

    if (!(root = tree->root))
        return NULL;
    node = root;
    keylen = strlen(key);


}

#if 0
/*
 * FUSE operations
 */

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

}

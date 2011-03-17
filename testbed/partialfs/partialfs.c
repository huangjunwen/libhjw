//#include <fuse.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

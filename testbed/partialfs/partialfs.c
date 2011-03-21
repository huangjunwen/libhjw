#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "radix.h"
#include "partialfs.h"

typedef struct file_handler_t {
    uint64_t fh;                    // the original
    path_operations_t * ops;
    off_t rel_off;                  // path's relative part offset
} file_handler_t;

static rdx_tree_t hier;

/* APIs
 */

void partialfs_init() {
    rdx_tree_init(&hier);
    // deny all by default
    partialfs_mount_path("/", &path_deny_ops);
}

int partialfs_mount_path(const char * path, path_operations_t * ops) {
    int err;
    rdx_node_t * node;

    if (!ops)
        return -1;

    node = rdx_tree_ensure(&tree, path, &err);
    if (err || !node)
        return -1;
    
    node->val = ops;
    return 0;

}

extern int partialfs_main(int argc, char * argv);

/* help functions
 */
static path_operations_t * get_path_ops(const char * path, 
        path_info_t * pi) {

    path_operations_t * ops;
    rdx_node_t * node;
    rdx_prefix_iter_t pfx_iter;

    if (path[0] != '/')
        return NULL;
    
    ops = NULL;
    node = rdx_prefix_iter_begin(&hier, path, &pfx_iter);
    while (node) {
        ops = (path_operations_t *)node->val;
        pi->rel_part = path + node->keylen;
        node = rdx_prefix_iter_next(&pfx_iter);
    }

    if (ops)
        pi->full_path = path;

    return ops;
}

/* fuse ops 
 */


#define SAVE_FH(fuse_file, file_handler) fuse_file->fh = (uint64_t)file_handler;

#define RESTORE_FH(path, fuse_file, path_info, file_handler) \
    file_handler = (file_handler_t *)((fuse_file)->fh); \
    path_info.full_path = path; \
    path_info.rel_part = path + file_handler->rel_off; \
    fuse_file->fh = file_handler->fh;


int partial_getattr(const char * path, struct stat * stbuf) {
    path_info_t pi;
    path_operations_t * ops;

    ops = get_path_ops(path, &pi);
    assert(ops);

    return (ops->getattr)(&pi, stbuf);
}

int partial_readlink(const char * path, char * buf, size_t sz) {
    path_info_t pi;
    path_operations_t * ops;

    ops = get_path_ops(path, &pi);
    assert(ops);

    return (ops->readlink)(&pi, buf, sz);
}

int partial_mknod(const char * path, mode_t mode, dev_t dev) {
    return EROFS;
}

int partial_mkdir(const char * path, mode_t mode) {
    return EROFS;
}

int partial_unlink(const char * path) {
    return EROFS;
}

int partial_rmdir(const char * path) {
    return EROFS;
}

int partial_symlink(const char * path, const char * link) {
    return EROFS;
}

int partial_rename(const char * path, const char * newpath) {
    return EROFS;
}

int partial_link(const char * path, const char * link) {
    return EROFS;
}

int partial_chmod(const char * path, mode_t mode) {
    return EROFS;
}

int partial_chown(const char * path, uid_t uid, gid_t gid) {
    return EROFS;
}

int partial_truncate(const char * path, off_t off) {
    return EROFS;
}

int partial_open(const char * path, struct fuse_file_info * fi) {

    path_info_t pi;
    path_operations_t * ops;
    int fd;
    file_handler_t * fh;
    static int allow_mask = ~(O_RDONLY | O_EXCL | O_NONBLOCK | 
        O_DSYNC | O_RSYNC | O_SYNC);
    
    if (fi->flags & allow_mask)
        return EROFS;

    ops = get_path_ops(path, &pi);
    assert(ops);

    fd = (ops->open)(&pi, fi);
    if (fd < 0)
        return fd;
    
    // overwrite fi->fh to store more data
    fh = (file_handler_t *)malloc(sizeof(file_handler_t));
    if (!fh) {
        (ops->release)(&pi, fi);
        return -1;
    }
    fh->fh = fi->fh;
    fh->ops = ops;
    fh->rel_off = pi.rel_part - pi.full_path;
    SAVE_FH(fi, fh);

    return fd;
}

int partial_read(const char * path, char * buf, size_t size, off_t off,
         struct fuse_file_info * fi) {

    path_info_t pi;
    file_handler_t * fh;
    int ret;

    RESTORE_FH(path, fi, pi, fh);
    ret = (fh->ops->read)(&pi, buf, size, off, fi);
    SAVE_FH(fi, fh);
    return ret;
}

int partial_write(const char * path, const char * buf, size_t size, off_t off,
          struct fuse_file_info * fi) {
    return EROFS;
}

int partial_statfs(const char * path, struct statvfs * stat) {
    path_info_t pi;
    path_operations_t * ops;

    ops = get_path_ops(path, &pi);
    assert(ops);

    return (ops->statfs)(&pi, stat);
}

int partial_flush(const char * path, struct fuse_file_info * fi) {
    return 0;
}

int partial_release(const char * path, struct fuse_file_info * fi) {
    path_info_t pi;
    file_handler_t * fh;
    int ret;

    RESTORE_FH(path, fi, pi, fh);
    ret = (fh->ops->release)(&pi, fi);
    free(fh);
    return ret;
}

int partial_fsync(const char * path, int datasync, struct fuse_file_info * fi) {
    return 0;
}

int partial_setxattr(const char * path, const char * name, const char * value, 
        size_t size, int flags) {
    return EROFS;
}

int partial_getxattr(const char * path, const char * name, char * value, size_t size) {
    path_info_t pi;
    path_operations_t * ops;

    ops = get_path_ops(path, &pi);
    assert(ops);

    return (ops->getxattr)(&pi, name, value, size);
}

int partial_listxattr(const char * path, char * list, size_t size) {
    path_info_t pi;
    path_operations_t * ops;

    ops = get_path_ops(path, &pi);
    assert(ops);

    return (ops->listxattr)(&pi, list, size);
}

int partial_removexattr(const char * path, const char * name) {
    return EROFS;
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


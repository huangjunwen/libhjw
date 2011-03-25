#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "radix.h"
#include "partialfs.h"

#define CHECK_PATH(path, path_len) \
    if (!(path_len)) (path_len) = strlen((path)); \
    if (!(path_len)) return -1; \
    if ((path)[0] != '/') return -1;

#define IS_FPATH(path, path_len) ((path)[(path_len) - 1] != '/')

/* path control 
 */
typedef struct path_ctrl_t {
    /* these attributes are used both by file path and dir path
     */
    // is it a file path (not ends with '/')
    int is_fpath;                   

    // for '/' and '/usr' path_level is 1
    // for '/usr/' and '/usr/local' path_level is 2
    int path_level;                     

    // is it allowed
    int allow;

    /* these attributes are used only by dir path
     */
    subfs_operations_t * subfs_ops;

    void * subfs_data;

} path_ctrl_t;

static rdx_tree_t hier_ctrl;

int pfs_init() {
    rdx_tree_init(&hier_ctrl);
    return pfs_mount_subfs("/", 1, 0, NULL, &default_subfs_ops);
}

static int _pfs_ctrl_path(const char * path, size_t path_len, int allow) {

    rdx_node_t * node;
    path_ctrl_t * pctl;
    const char * p;
    int err;

    CHECK_PATH(path, path_len);

    // insert
    node = rdx_tree_ensure(&hier_ctrl, path, path_len, &err);
    if (err)
        return -1;      // not enough memory

    pctl = (path_ctrl_t *)node->val;
    if (!pctl) {
        pctl = (path_ctrl_t *)malloc(sizeof(path_ctrl_t));
        if (!pctl)
            return -1;
        pctl->subfs_ops = NULL;
        pctl->subfs_data = NULL;
    }

    // fill
    pctl->is_fpath = IS_FPATH(path, path_len);
    pctl->path_level = 0;
    p = path;
    while (p < path + path_len) {
        if (*p++ == '/')
            ++pctl->path_level;
    }
    pctl->allow = allow ? 1 : 0;
    node->val = (void *)pctl;
    return 0;
}

int pfs_allow_path(const char * path, size_t path_len) {
    return _pfs_ctrl_path(path, path_len, 1);
}

int pfs_deny_path(const char * path, size_t path_len) {
    return _pfs_ctrl_path(path, path_len, 0);
}

int pfs_mount_subfs(const char * dpath, size_t dpath_len, 
        int subfs_argc,
        const char * subfs_argv[],
        subfs_operations_t * subfs_ops) {

    rdx_node_t * node;
    path_ctrl_t * pctl;
    int r;
    void * subfs_data;

    CHECK_PATH(dpath, dpath_len);

    if (IS_FPATH(dpath, dpath_len))
        return -1;

    if (pfs_allow_path(dpath, dpath_len) < 0)
        return -1;

    node = rdx_tree_lookup(&hier_ctrl, dpath, dpath_len, &r);
    assert(node);
    pctl = (path_ctrl_t *)node->val;
    assert(pctl);

    // unmount the previous subfs if exists
    if (pctl->subfs_ops && pctl->subfs_ops->fini)
        (pctl->subfs_ops->fini)(pctl->subfs_data);
    pctl->subfs_ops = NULL;
    pctl->subfs_data = NULL;

    // init subfs
    subfs_data = NULL;
    if (subfs_ops->init) {
        r = (subfs_ops->init)(subfs_argc, subfs_argv, &subfs_data);
        if (r < 0)
            return -1;
    }
    pctl->subfs_ops = subfs_ops;
    pctl->subfs_data = subfs_data;
    return 0;

}

// iternal used only
static inline int _prefix_is_path_prefix(rdx_node_t * pfx, 
        const char * path) {

    path_ctrl_t * pctl;
    char e;

    pctl = (path_ctrl_t *)pfx->val;
    if (!pctl->is_fpath)
        return 1;
    // '/usr' is a prefix of '/usr1/local' 
    // but not a path prefix of it
    e = path[pfx->keylen];
    if (e == '/' || e == '\0')
        return 1;
    return 0;
}

static rdx_node_t * _path_prefix_iter_begin(const char * path, 
        size_t path_len,
        rdx_prefix_iter_t * iter) {

    rdx_node_t * pfx;

    pfx = rdx_prefix_iter_begin(&hier_ctrl, path, path_len, iter);
    while (pfx) {
        if (_prefix_is_path_prefix(pfx, path))
            return pfx;
        pfx = rdx_prefix_iter_next(iter);
    }
    return NULL;
}

static rdx_node_t * _path_prefix_iter_next(rdx_prefix_iter_t * iter) {

    rdx_node_t * pfx;
    const char * path;

    path = iter->key;
    while (1) {
        pfx = rdx_prefix_iter_next(iter);
        if (!pfx)
            break;
        if (_prefix_is_path_prefix(pfx, path))
            return pfx;
    }
    return NULL;
}

int pfs_get_path_visibility(const char * path, size_t path_len,
        const char ** subfs_path,
        subfs_operations_t ** subfs_ops,
        void ** subfs_data) {

    rdx_prefix_iter_t iter;
    rdx_node_t * pfx;
    path_ctrl_t * pctl;
    int allow, next_level;
    int remain;

    CHECK_PATH(path, path_len);

    // '/' should always here
    pfx = _path_prefix_iter_begin(path, path_len, &iter);
    assert(pfx && pfx->key[0] == '/' && pfx->keylen == 1);
    pctl = (path_ctrl_t *)pfx->val;
    remain = path_len - pfx->keylen;

    allow = pctl->allow;
    next_level = pctl->path_level;
    *subfs_path = path;
    *subfs_ops = pctl->subfs_ops;
    *subfs_data = pctl->subfs_data;

    while (1) {
        pfx = _path_prefix_iter_next(&iter);
        if (!pfx)
            break;
        pctl = (path_ctrl_t *)pfx->val;
        remain = path_len - pfx->keylen;

        // we are under an invisible dir
        // and at least one intermediate path is not in 
        // hier_ctrl so by default its invisible
        if (!allow && pctl->path_level != next_level)
            return 0;

        // file path
        if (pctl->is_fpath) {
            // explicit deny
            if (!pctl->allow)
                return 0;
            ++next_level;
            continue;
        }

        // dir path
        allow = pctl->allow;
        next_level = pctl->path_level;
        if (pctl->subfs_ops) {
            *subfs_path = path + pfx->keylen - 1;
            *subfs_ops = pctl->subfs_ops;
            *subfs_data = pctl->subfs_data;
        }
    }

    // in a visible dir
    // or no remain
    // or remain == 1, the only case is ('/usr', '/usr/')
    if (allow || remain <= 1)
        return 1;

    return 0;

}


subfs_operations_t default_subfs_ops = {
    .init = NULL,
    .fini = NULL
};


/* fuse ops 
 */

#if 0


int partial_getattr(const char * path, struct stat * stbuf) {
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

#endif


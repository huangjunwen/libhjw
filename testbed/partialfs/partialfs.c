#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "radix.h"
#include "partialfs.h"

// path control
typedef struct path_ctl_t {
    /* both is_fpath and path_level can be
     * calculated from the key itself
     * but record here for convience
     */
    // is it a file path (not ends with '/')
    int is_fpath;                   
    // for '/' and '/usr' path_level is 1
    // for '/usr/' and '/usr/local' path_level is 2
    int path_level;                     

    /* visibility
     */
    int vis;
    /* if this path is dir path and visble one can
     * specify another ops
     */
    path_operations_t * ops;
} path_ctl_t;

static rdx_tree_t hier_ctl;

void partialfs_init() {
    int r;
    char * err_msg;

    rdx_tree_init(&hier_ctl);
    r = dcl_path_visibility("/", 1, 0, 0, &err_msg);
    assert(r == 0);
}

int dcl_path_visibility(const char * path, size_t path_len,
        int vis,
        path_operations_t * ops,
        char ** err_msg) {
    
    rdx_node_t * node;
    path_ctl_t * pctl;
    const char * p, * e;
    int err;
    int is_fpath, path_level;

    err_msg = NULL;

    /* check args
     */
    path_len = path_len ? path_len : strlen(path);
    if (!path_len) {
        asprintf(err_msg, "empty path");
        return -1;
    }
    if (path[0] != '/') {
        asprintf(err_msg, "path must starts with '/'");
        return -1;
    }

    is_fpath = (path[path_len - 1] != '/');
    if (ops && (!vis || is_fpath)) {
        asprintf(err_msg, "only visible dir path can use another path"
                " operations");
        return -1;
    }
    
    p = path;
    e = path + path_len;
    path_level = 0;
    while (p < e) {
        if (*p == '/')
            ++path_level;
        ++p;
    }
    
    /* insert into the radix tree
     */
    node = rdx_tree_ensure(&hier_ctl, path, path_len, &err);
    if (err) {
        asprintf(err_msg, "not enough mem (rdx node)");
        return -1;
    }

    pctl = node->val ? (path_ctl_t *)node->val : 
        (path_ctl_t *)malloc(sizeof(path_ctl_t));
    if (!pctl) {
        asprintf(err_msg, "not enough mem (path ctl)");
        return -1;
    }

    pctl->is_fpath = is_fpath;
    pctl->path_level = path_level;
    pctl->vis = vis;
    pctl->ops = ops;
    node->val = (void *)pctl;
    return 0;

}

// iternal used only
static inline int _prefix_is_path_prefix(rdx_node_t * pfx, 
        const char * path) {

    path_ctl_t * pctl;
    char e;

    pctl = (path_ctl_t *)pfx->val;
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

    pfx = rdx_prefix_iter_begin(&hier_ctl, path, path_len, iter);
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


int get_dpath_visibility(const char * path, size_t path_len, 
        path_operations_t ** pops) {

    rdx_prefix_iter_t iter;
    rdx_node_t * pfx;
    path_ctl_t * pctl;
    int dir_vis;
    path_operations_t * ops;

    if (!path_len)
        path_len = strlen(path);
    if (!path_len)
        return -1;
    if (path[0] != '/')
        return -1;

    // must be a dpath
    if (path[path_len - 1] != '/')
        return -1;

    pfx = _path_prefix_iter_begin(path, path_len, &iter);
    assert(pfx);
    pctl = (path_ctl_t *)pfx->val;
    assert(!pctl->is_fpath);

    dir_vis = pctl->vis;
    ops = pctl->ops;

    // get the longest dpath's visibility and operations
    while (1) {
        pfx = _path_prefix_iter_next(&iter);
        if (!pfx)
            break;
        pctl = (path_ctl_t *)pfx->val;

        if (pctl->is_fpath)
            continue;

        dir_vis = pctl->vis;
        if (pctl->ops)
            ops = pctl->ops;
    }

    if (!dir_vis)
        return 0;
    if (pops)
        *pops = ops;
    return 1;
}


int get_fpath_visibility(const char * path, size_t path_len,
        path_operations_t ** pops) {

    rdx_prefix_iter_t iter;
    rdx_node_t * pfx;
    path_ctl_t * pctl;
    int vis, dir_vis, expect_level;
    path_operations_t * ops;
    const char * remain;

    if (!path_len)
        path_len = strlen(path);
    if (!path_len)
        return -1;
    if (path[0] != '/')
        return -1;

    // must be a fpath
    if (path[path_len - 1] == '/')
        return -1;

    // '/' should always here
    pfx = _path_prefix_iter_begin(path, path_len, &iter);
    assert(pfx);
    pctl = (path_ctl_t *)pfx->val;
    assert(!pctl->is_fpath);

    dir_vis = pctl->vis;
    ops = pctl->ops;
    expect_level = pctl->path_level;
    remain = path + pfx->keylen;

    while (1) {
        pfx = _path_prefix_iter_next(&iter);
        if (!pfx)
            break;
        pctl = (path_ctl_t *)pfx->val;
        remain = path + pfx->keylen;
        vis = pctl->vis;

        if (dir_vis) {
            // file path
            if (pctl->is_fpath) {
                if (!pctl->vis)
                    goto INVIS;
                continue;
            }

            // dir path
            if (pctl->ops)
                ops = pctl->ops;
            dir_vis = vis;
            expect_level = pctl->path_level;
        }
        else {
            if (pctl->path_level != expect_level)
                goto INVIS;

            // file path
            if (pctl->is_fpath) {
                if (!pctl->vis)
                    goto INVIS;
                ++expect_level;
                continue;
            }

            // dir path
            if (pctl->ops)
                ops = pctl->ops;
            dir_vis = vis;
        }
    }

    // the last dir path is visible
    if (dir_vis)
        goto VIS;

    // the last is a file path and visible and no remain
    if (vis && remain[0] == '\0')
        goto VIS;

    goto INVIS;

VIS:
    if (pops)
        *pops = ops;
    return 1;
INVIS:
    return 0;
}


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


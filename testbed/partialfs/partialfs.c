#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "radix.h"
#include "partialfs.h"

/* a path is a 'file path' if it not ends with '/', such:
 *      /usr/local
 *      /home
 * a path is a 'dir path' if it ends with '/', such:
 *      /
 *      /home/
 *
 * a 'file path' is visible means:
 *      the node pointed by the path is visible
 * a 'dir path' is visible means:
 *      its sub nodes (recursivly) are visible by default
 *      for example:
 *          if /home/ is visible
 *          then /home/jayven, /home/jayven/abc is visible by default
 */


// path control
typedef struct path_ctl_t {
    int is_fpath;                   // is it a file path (or else dir path)
    int vis;
    path_operations_t * ops;        // only for dir path
    void * args;
} path_ctl_t;

static rdx_tree_t hier_ctl;

/* APIs
 */

int dcl_path_visibility(const char * path, size_t path_len,
        int vis,
        path_operations_t * ops,
        void * args,
        char ** err_msg) {
    
    rdx_node_t * node;
    path_ctl_t * pctl;
    const char * e;
    int err;

    err_msg = NULL;

    if (!path_len)
        path_len = strlen(path);
    if (!path_len) {
        asprintf(err_msg, "empty path");
        return -1;
    }
    if (path[0] != '/') {
        asprintf(err_msg, "path must starts with '/'");
        return -1;
    }

    if (!vis) {
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

        pctl->is_fpath = (path[path_len - 1] != '/');
        pctl->vis = 0;
        pctl->ops = NULL;
        pctl->args = NULL;
        node->val = (void *)pctl;
        return 0;
    }

    // if ops or args, path should convert to a dir path
    if (ops || args) {
        if (path[path_len - 1] != '/') {
            ++path_len;
            char dpath[path_len + 1];
            strncpy(dpath, path, path_len - 1);
            dpath[path_len - 1] = '/';
            dpath[path_len] = '\0';
            path = dpath;
        }
    }

    // also add each of its parent file path
    e = path;
    while (*e) {
        e = strchr(e + 1, '/');
        if (!e)
            break;
        // already visible
        if (get_path_visibility(path, e - path, NULL, NULL))
            continue;
    }



}

int get_path_visibility(const char * path, size_t path_len, 
        path_operations_t ** pops,
        void ** pargs) {

    int vis;
    path_operations_t * ops;
    void * args;
    char c;
    path_ctl_t * pctl;
    rdx_node_t * node;
    rdx_prefix_iter_t pfx_iter;

    vis = 0;
    ops = NULL;
    args = NULL;
    node = rdx_prefix_iter_begin(&hier_ctl, path, path_len, &pfx_iter);
    while (node) {
        pctl = (path_ctl_t *)node->val;
        if (pctl->is_fpath) {
            // make sure a prefix is a parent file path, for example:
            //      /usr is a prefix of /usr1/local but not a parent file path
            if (path[node->keylen] == '/' || node->keylen == path_len) {
                // if any parent path is invisible explicit
                // the path is invisible
                if (!pctl->vis)
                    return 0;
            }
        }
        else {
            // inherit the longest parent dir path's visibility and operators
            vis = pctl->vis;
            if (pctl->ops)
                ops = pctl->ops;
            if (pctl->args)
                args = pctl->args;
        }
        node = rdx_prefix_iter_next(&pfx_iter);
    }

    if (pops)
        *pops = ops;
    if (pargs)
        *pargs = args;
    return vis;
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


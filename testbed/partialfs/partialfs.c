#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include "radix.h"
#include "partialfs.h"

#define CHECK_PATH(path, path_len) \
    if (!(path_len)) (path_len) = strlen((path)); \
    if (!(path_len)) return -1; \
    if ((path)[0] != '/') return -1;

#define IS_FPATH(path, path_len) ((path)[(path_len) - 1] != '/')
#define IS_DPATH(path, path_len) ((path)[(path_len) - 1] == '/')

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

} path_ctrl_t;

static rdx_tree_t hier_ctrl;

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

int pfs_get_path_visibility(const char * path, 
        size_t path_len) {

    rdx_prefix_iter_t iter;
    rdx_node_t * pfx;
    path_ctrl_t * pctl;
    int default_allow, expect_level;
    int remain;

    CHECK_PATH(path, path_len);

    // convert a dir path to file path
    if (IS_DPATH(path, path_len)) {
        --path_len;
        // root always visible
        if (!path_len)
            return 1;
    }

    // '/' should always here
    pfx = _path_prefix_iter_begin(path, path_len, &iter);
    assert(pfx && pfx->key[0] == '/' && pfx->keylen == 1);
    pctl = (path_ctrl_t *)pfx->val;

    remain = path_len - pfx->keylen;
    default_allow = pctl->allow;
    expect_level = pctl->path_level;
    assert(expect_level == 1);

    while (1) {
        pfx = _path_prefix_iter_next(&iter);
        if (!pfx)
            break;
        pctl = (path_ctrl_t *)pfx->val;
        remain = path_len - pfx->keylen;

        // we are under an invisible dir
        // and at least one intermediate path is not in 
        // hier_ctrl so by default its invisible
        if (!default_allow && pctl->path_level != expect_level)
            return 0;

        // file path
        if (pctl->is_fpath) {
            // explicit deny
            if (!pctl->allow)
                return 0;
            ++expect_level;
            continue;
        }

        // dir path
        default_allow = pctl->allow;
        expect_level = pctl->path_level;
    }

    // in a visible dir
    // or no remain
    if (default_allow|| !remain)
        return 1;

    return 0;

}

static inline int pfs_err() {
    return -errno;
}

typedef union {
    int i;
    void * ptr;
    uint64_t u64;
} fh_t;

/* fuse ops 
 */


int partialfs_getattr(const char * path, struct stat * stbuf) {
    int r;

    if (!pfs_get_path_visibility(path, 0))
        return -ENOENT;

    r = lstat(path, stbuf);
    if (r < 0)
        r = pfs_err();
    return r;
}

int partialfs_readlink(const char * path, char * buf, size_t sz) {
    int r;

    if (!pfs_get_path_visibility(path, 0))
        return -ENOENT;
    r = readlink(path, buf, sz - 1);
    if (r < 0)
        r = pfs_err();
    else {
        buf[r] = '\0';
        r = 0;
    }
    return r;
}

int partialfs_mknod(const char * path, mode_t mode, dev_t dev) {
    return -EROFS;
}

int partialfs_mkdir(const char * path, mode_t mode) {
    return -EROFS;
}

int partialfs_unlink(const char * path) {
    return -EROFS;
}

int partialfs_rmdir(const char * path) {
    return -EROFS;
}

int partialfs_symlink(const char * path, const char * link) {
    return -EROFS;
}

int partialfs_rename(const char * path, const char * newpath) {
    return -EROFS;
}

int partialfs_link(const char * path, const char * link) {
    return -EROFS;
}

int partialfs_chmod(const char * path, mode_t mode) {
    return -EROFS;
}

int partialfs_chown(const char * path, uid_t uid, gid_t gid) {
    return -EROFS;
}

int partialfs_truncate(const char * path, off_t off) {
    return -EROFS;
}

int partialfs_open(const char * path, struct fuse_file_info * fi) {

    static int allow_mask = ~(O_RDONLY | O_EXCL 
        | O_NOCTTY
        | O_NONBLOCK
        | O_NDELAY
        | O_ASYNC
#ifdef __USE_GNU
        | O_DIRECTORY
        | O_NOFOLLOW
        | O_NOATIME
        | O_CLOEXEC
#endif
        );
    int fd;

    if (fi->flags & allow_mask)
        return -EROFS;
    if (!pfs_get_path_visibility(path, 0))
        return -ENOENT;
    fd = open(path, fi->flags);
    if (fd < 0)
        fd = pfs_err();
    else
        fi->fh = (uint64_t)fd;
    return fd;
}

int partialfs_read(const char * path, char * buf, size_t size, off_t off,
         struct fuse_file_info * fi) {
    int r;

    r = pread(fi->fh, buf, size, off);
    if (r < 0)
        r = pfs_err();
    return r;
}

int partialfs_write(const char * path, const char * buf, size_t size, off_t off,
          struct fuse_file_info * fi) {
    return -EROFS;
}

int partialfs_statfs(const char * path, struct statvfs * stat) {
    return -ENOSYS;
}

int partialfs_flush(const char * path, struct fuse_file_info * fi) {
    return 0;
}

int partialfs_release(const char * path, struct fuse_file_info * fi) {
    return close(fi->fh);
}

int partialfs_fsync(const char * path, int datasync, struct fuse_file_info * fi) {
    return 0;
}

int partialfs_opendir(const char * path, struct fuse_file_info * fi) {
    DIR * dp;
    fh_t * pfh;

    if (!pfs_get_path_visibility(path, 0))
        return -ENOENT;
    dp = opendir(path);
    if (dp == NULL)
        return pfs_err();
    pfh = (fh_t *)&fi->fh;
    pfh->ptr = (void *)dp;
    
    return 0;
}

int partialfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, 
        off_t off,
        struct fuse_file_info * fi) {
    DIR * dp;
    struct dirent * de;
    const char * dname;
    size_t path_len, dname_len, sub_path_len;

    // make it a file path
    path_len = strlen(path);
    if (IS_DPATH(path, path_len))
        --path_len;

    dp = (DIR *)((fh_t)fi->fh).ptr;

    errno = 0;

    while ((de = readdir(dp)) != NULL) {
        if (strcmp(dname, ".") != 0 && strcmp(dname, "..") != 0) {
            dname = de->d_name;
            dname_len = strlen(dname);

            // sub_path: "path/dname"
            sub_path_len = path_len + dname_len + 1;
            char sub_path[sub_path_len + 1];

            strncpy(sub_path, path, path_len);
            sub_path[path_len] = '/';
            strncpy(sub_path + path_len + 1, dname, dname_len);
            sub_path[sub_path_len - 1] = '\0';

            // check visibility
            if (!pfs_get_path_visibility(sub_path, sub_path_len))
                continue;
        }
        if (filler(buf, dname, NULL, 0) != 0)
            return -ENOMEM;
    }

    // err
    if (errno > 0)
        return pfs_err();

    return 0;

}

int partialfs_releasedir(const char * path, struct fuse_file_info * fi) {
    int r;

    r = closedir((DIR *)((fh_t)fi->fh).ptr);
    if (r < 0)
        return pfs_err();
    return r;
}

int partialfs_fsyncdir(const char * path, int datasync, struct fuse_file_info * fi) {
    return 0;
}

void * partialfs_init(struct fuse_conn_info * conn) {
    rdx_tree_init(&hier_ctrl);
    pfs_deny_path("/", 1);

    return NULL;
}

void partialfs_destroy(void * userdata) {
    rdx_tree_fini(&hier_ctrl);
}

int partialfs_access(const char * path, int mask) {
    int r;

    if (!pfs_get_path_visibility(path, 0))
        return -ENOENT;
    r = access(path, mask);
    if (r < 0)
        return pfs_err();
    return r;
}

int partialfs_create(const char * path, mode_t mode, struct fuse_file_info * fi) {
    return -EROFS;
}

int partialfs_ftruncate(const char * path, off_t off, struct fuse_file_info * fi) {
    return -EROFS;
}

int partialfs_fgetattr(const char * path, struct stat * statbuf, struct fuse_file_info * fi) {
    int r;

    r = fstat(fi->fh, statbuf);
    if (r < 0)
        return pfs_err();
    return r;
}

struct fuse_operations partialfs_oper = {
    .getattr        = partialfs_getattr,
    .readlink       = partialfs_readlink,
    .mknod          = partialfs_mknod,
    .mkdir          = partialfs_mkdir,
    .unlink         = partialfs_unlink,
    .rmdir          = partialfs_rmdir,
    .symlink        = partialfs_symlink,
    .rename         = partialfs_rename,
    .link           = partialfs_link,
    .chmod          = partialfs_chmod,
    .chown          = partialfs_chown,
    .truncate       = partialfs_truncate,
    .open           = partialfs_open,
    .read           = partialfs_read,
    .write          = partialfs_write,
    .statfs         = partialfs_statfs,
    .flush          = partialfs_flush,
    .release        = partialfs_release,
    .fsync          = partialfs_fsync,
#if 0
    .setxattr       = partialfs_setxattr,
    .getxattr       = partialfs_getxattr,
    .listxattr      = partialfs_listxattr,
    .removexattr    = partialfs_removexattr,
#endif
    .opendir        = partialfs_opendir,
    .readdir        = partialfs_readdir,
    .releasedir     = partialfs_releasedir,
    .fsyncdir       = partialfs_fsyncdir,
    .init           = partialfs_init,
    .destroy        = partialfs_destroy,
    .access         = partialfs_access,
    .create         = partialfs_create,
    .ftruncate      = partialfs_ftruncate,
    .fgetattr       = partialfs_fgetattr
};



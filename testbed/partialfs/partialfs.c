#include <stdlib.h>
#include <string.h>
#include "partialfs.h"

extern path_operations_t default_path_operations;

/* hier_t is organized hierarchically
 */
#define VIS_INVISIBLE (0)
#define VIS_INHERIT (1)
#define VIS_VISIBLE (2)

struct hier_node_t {
    rdx_tree_t sub_paths;
    int visibility;
    path_operations_t * ops;    // when this sub path is visible and the remaining
                                // sub path is not found in sub_paths
                                // operations will be handle to this 'ops'
};


static hier_node_t * hier_node_create(int visibility, path_operations_t * ops) {
    hier_node_t * ret;
    ret = (hier_node_t *)malloc(sizeof(hier_node_t));
    if (!ret)
        return NULL;
    rdx_tree_init(&ret->sub_paths);
    ret->visibility = visibility;
    ret->ops = ops ? ops : &default_path_operations;
    return ret;
}

static void hier_node_destory(hier_node_t * node) {
    rdx_iter_t iter;
    rdx_node_t * sub;

    sub = rdx_iter_begin(&iter, &node->sub_paths);
    while (sub) {
        hier_node_destory((hier_node_t *)sub->val);
        sub = rdx_iter_next(&iter);
    }

    rdx_tree_fini(&node->sub_paths);
}

void * hier_create(int visible, path_operations_t * ops) {
    if (visible != VIS_INVISIBLE && visible != VIS_VISIBLE)
        return NULL;
    return (void *)hier_node_create(visible, ops);
}

void hier_destroy(void * hier) {
    hier_node_destory((hier_node_t *)hier);
}

int hier_add_path(void * hier_root, const char * full_path, int visible,
        path_operations_t * ops) {

    hier_node_t * hier_node;
    const char * full_path_end;
    char * sub_begin, * sub_end;
    size_t full_path_len;
    rdx_node_t * n;
    int err;
    

    if (full_path[0] != '/')
        return -1;
    if (visible != VIS_INVISIBLE && visible != VIS_VISIBLE)
        return -1;
    ops = ops ? ops : &default_path_operations;

    hier_node = (hier_node_t *)hier_root;
    full_path_end = full_path + strlen(full_path);

    if (visible) {
        sub_end = full_path;
        // make each sub path visible
        do {
            sub_begin = sub_end + 1;
            sub_end = strchr(sub_begin, '/');
            if (!sub_end)
                sub_end = full_path_end;

            // empty
            if (sub_begin >= sub_end)
                continue;

            n = rdx_tree_ensure(&hier_node->sub_paths, sub_begin,
                    sub_end - sub_begin, 
                    &err);
            if (err || !n)
                return -1;

            // make sure the sub node exists
            if (!n->val) {
                n->val = hier_node_create(VIS_VISIBLE, ops);
                if (!n->val)
                    return -1;
            }
            else {
                n->val.visibility = VIS_VISIBLE;
                n->val.ops = ops;
            }
        } while (sub_end != full_path_end);
    } else {
    }

}

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

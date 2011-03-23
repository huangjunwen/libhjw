#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#include <stddef.h>
#include <stdint.h>
#include <fuse.h>


/* mostly the same as READ operations in fuse_operations
 * but replace `const char *` with `path_info_t` 
 */
typedef struct path_operations_t {
    int (*getattr) (path_info_t *, struct stat *);
	int (*readlink) (path_info_t *, char *, size_t);
	int (*open) (path_info_t *, struct fuse_file_info *);
	int (*read) (path_info_t *, char *, size_t, off_t, struct fuse_file_info *);
	int (*release) (path_info_t *, struct fuse_file_info *);
	int (*statfs) (path_info_t *, struct statvfs *);
	int (*getxattr) (path_info_t *, const char *, char *, size_t);
	int (*listxattr) (path_info_t *, char *, size_t);
	int (*opendir) (path_info_t *, struct fuse_file_info *);
	int (*readdir) (path_info_t *, void *, fuse_fill_dir_t, off_t,
			struct fuse_file_info *);
	int (*releasedir) (path_info_t *, struct fuse_file_info *);
	int (*access) (path_info_t *, int);
	int (*fgetattr) (path_info_t *, struct stat *, struct fuse_file_info *);
} path_operations_t;

/* APIs
 */

int dcl_path_visibility(const char * path, size_t path_len,
        int vis,
        path_operations_t * ops,
        void * args,
        char ** err_msg);


int get_path_visibility(const char * path, size_t path_len, 
        path_operations_t ** pops,
        void ** pargs);

#endif

#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#include <stddef.h>
#include <stdint.h>
#include <fuse.h>

/* A path info is 2 part path
 *  full_path: the origin path passed in
 *  absolute part: from root directory on to some sub path
 *  relative part: full_path - absolute part
 */

typedef struct path_info_t {
    const char * full_path;
    const char * rel_part;
} path_info_t;

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

/* two builtin operations
 */
extern path_operations_t path_default_ops;

extern path_operations_t path_deny_ops;

/* APIs
 */
extern void partialfs_init(void);

// return -1 on err and 0 for ok
extern int partialfs_mount_path(const char * path, path_operations_t * ops);

extern int partialfs_main(int argc, char * argv);


#endif

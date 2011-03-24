#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#include <stddef.h>
#include <stdint.h>
//#include <fuse.h>


/* mostly the same as READ operations in fuse_operations
 * but replace `const char *` with `path_info_t` 
 */
typedef struct path_operations_t {
#if 0
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
#else
    int used;
#endif
} path_operations_t;


/* a path is a 'file path' if it doesn't ends with '/', such:
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
extern void partialfs_init(void);

extern int dcl_path_visibility(const char * path, size_t path_len,
        int vis,
        path_operations_t * ops,
        char ** err_msg);

extern int get_dpath_visibility(const char * path, size_t path_len, 
        path_operations_t ** pops);

extern int get_fpath_visibility(const char * path, size_t path_len,
        path_operations_t ** pops);

#endif

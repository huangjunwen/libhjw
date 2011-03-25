#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#include <stddef.h>
#include <stdint.h>
//#include <fuse.h>


/* mostly the same as READ operations in fuse_operations
 */
typedef struct subfs_operations_t {
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
#endif

    int (*init) (int argc, const char * argv[], void ** data);
    void (*fini) (void * data);

} subfs_operations_t;


/* export symbols
 */
extern subfs_operations_t default_subfs_ops;


/* API
 */

/* init partialfs
 * return 0 on success, -1 if failed
 */
extern int pfs_init(void);

/* a path is a 'file path' if it doesn't ends with '/', such:
 *      /usr/local
 *      /home
 * a path is a 'dir path' if it ends with '/', such:
 *      /
 *      /home/
 *
 * a 'file path' is 'allow' means:
 *      the path is visible if its dir is visible
 *
 * a 'dir path' is 'allow' means:
 *      its sub paths (recursivly) are allowed by default
 *      for example:
 *          if /home/ is allow
 *          then /home/jayven, /home/jayven/abc is visible by default
 * 
 * return 0 on success, -1 if failed
 */
extern int pfs_allow_path(const char * path, size_t path_len);

/* a 'file path' is 'deny' means:
 *      the path is invisible event its dir is visible
 *
 * a 'dir path' is 'deny' means:
 *      its sub paths (recursivly) are deny by default
 *
 * return 0 on success, -1 if failed
 */
extern int pfs_deny_path(const char * path, size_t path_len);

/* a subfs can only mount at a dpath
 * it also implies 'pfs_allow_path(dpath, dpath_len)'
 *
 * return 0 on success, -1 if failed
 */
extern int pfs_mount_subfs(const char * dpath, size_t dpath_len, 
        int subfs_argc,
        const char * subfs_argv[],
        subfs_operations_t * subfs_ops);

/* get a path's visibility
 *
 * return 1 if it is visible, 0 if invisible and -1 on err
 * when the path is visible, subfs_* will be filled
 * subfs_path points to path's memory so no need to free
 */
extern int pfs_get_path_visibility(const char * path, size_t path_len,
        const char ** psubfs_path,
        subfs_operations_t ** psubfs_ops,
        void ** psubfs_data);

#endif

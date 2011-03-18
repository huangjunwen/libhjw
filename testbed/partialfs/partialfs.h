#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#include <stddef.h>
#include <stdint.h>
#include <fuse.h>
#include "radix.h"

/* A path info is 2 part path
 *  full_path: the origin path passed in
 *  absolute part: from root directory on to some sub path
 *  relative part: full_path - absolute part
 */
#define ABS_PART(pi) ((pi)->full_path)
#define ABS_LEN(pi) ((pi)->abs_len)
#define REL_PART(pi) ((pi)>full_path[(pi)->abs_len])
#define REL_LEN(pi) ((pi)->full_len - (pi)->abs_len)

typedef struct path_info_t {
    const char * full_path;
    size_t full_len;
    size_t abs_len;
} path_info_t;

/* mostly the same as fuse_operations
 * but replace `const char *` with `path_info_t` 
 * and `fuse_file_info` with `uint64_t`
 * XXX: only READ operations are allowed
 */
typedef struct path_operations_t {
    int (*getattr) (path_info_t *, struct stat *);
	int (*readlink) (path_info_t *, char *, size_t);
    /* why don't use fuse_file_info directly?
     *  since i need to fill it myself
     */
	int (*open) (path_info_t * path, int flags, unsigned int * direct_io,
            unsigned int * keep_cache,
            unsigned int * nonseekable,
            uint64_t * fh);
	int (*read) (path_info_t *, char *, size_t, off_t, uint64_t fh);
	int (*release) (path_info_t *, uint64_t fh);
	int (*statfs) (path_info_t *, struct statvfs *);
	int (*getxattr) (path_info_t *, const char *, char *, size_t);
	int (*listxattr) (path_info_t *, char *, size_t);
	int (*opendir) (path_info_t * path, unsigned int * keep_cache,
            unsigned int * nonseekable,
            uint64_t * fh);
	int (*readdir) (path_info_t *, const char ** pname, 
            const struct stat ** pstbuf, 
            off_t off, 
            uint64_t fh);
	int (*releasedir) (path_info_t, uint64_t fh);
	int (*access) (path_info_t *, int);
} path_operations_t;

/* visible 0 or 1 (root dir must be explict visible or invisible)
 * ops can be user supplied or NULL, in the later case the default
 * operations is used, that is: directly map to the underly fs
 */
extern void * hier_create(int visible, path_operations_t * ops);

extern void hier_destroy(void * hier);

/* similar to hier_create but to add a sub path
 */
extern int hier_add_path(void * hier, const char * full_path, int visible,
        path_operations_t * ops);


/* the main operations
 */
extern struct fuse_operations partialfs_ops;

#endif

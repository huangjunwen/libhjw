#ifndef _PARTIALFS_H_
#define _PARTIALFS_H_

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stddef.h>
#include <stdint.h>

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

/* get a path's visibility
 *
 * return 1 if it is visible, 0 if invisible and -1 on err
 */
extern int pfs_get_path_visibility(const char * path, 
        size_t path_len); 

extern struct fuse_operations partialfs_oper;

#endif

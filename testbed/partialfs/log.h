#ifndef _LOG_H_
#define _LOG_H_

#define PFS_LOG_LVL_CRIT (4)
#define PFS_LOG_LVL_ERR (3)
#define PFS_LOG_LVL_WARNING (2)
#define PFS_LOG_LVL_INFO (1)
#define PFS_LOG_LVL_DEBUG (0)

// define global log level
// only log greater or equal LOG_LEVEL will be log down
#ifndef PFS_LOG_LEVEL
#define PFS_LOG_LEVEL PFS_LOG_LVL_DEBUG
#endif

extern void pfs_log_init(void);

extern void pfs_log_debug(const char * format, ...);

extern void pfs_log_info(const char * format, ...);

extern void pfs_log_warning(const char * format, ...);

extern void pfs_log_err(const char * format, ...);

extern void pfs_log_crit(const char * format, ...);

#endif

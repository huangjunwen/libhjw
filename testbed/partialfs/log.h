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

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_DEBUG
extern void _pfs_log_debug(const char * format, ...);
#define pfs_log_debug(args) _pfs_log_debug args
#else
#define pfs_log_debug(args)
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_INFO
extern void _pfs_log_info(const char * format, ...);
#define pfs_log_info(args) _pfs_log_info args
#else
#define pfs_log_info(args)
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_WARNING
extern void _pfs_log_warning(const char * format, ...);
#define pfs_log_warning(args) _pfs_log_warning args
#else
#define pfs_log_warning(args)
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_ERR
extern void _pfs_log_err(const char * format, ...);
#define pfs_log_err(args) _pfs_log_err args
#else
#define pfs_log_err(args)
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_CRIT
extern void _pfs_log_crit(const char * format, ...);
#define pfs_log_crit(args) _pfs_log_crit args
#else
#define pfs_log_crit(args)
#endif

#endif

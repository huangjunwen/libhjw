#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

void pfs_log_init() {
    openlog("partialfs", LOG_PID | LOG_NDELAY, 0);
}

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_DEBUG
void _pfs_log_debug(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(LOG_DEBUG, format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_INFO
void _pfs_log_info(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(LOG_INFO, format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_WARNING
void _pfs_log_warning(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(LOG_WARNING, format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_ERR
void _pfs_log_err(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(LOG_ERR, format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_CRIT
void _pfs_log_crit(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(LOG_CRIT, format, ap);
    va_end(ap);
}
#endif

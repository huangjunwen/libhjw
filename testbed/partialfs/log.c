#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include "log.h"

#define LOG_MAX_LEN (512)

void pfs_log_init() {
    openlog("partialfs", LOG_PID | LOG_NDELAY, 0);
}

static inline void _pfs_vlog(int log_lvl, const char * prefix, 
        const char * format, va_list ap) {
    char buff[LOG_MAX_LEN + 1];
    int sz;

    buff[LOG_MAX_LEN - 1] = '\0';
    sz = vsnprintf(buff, LOG_MAX_LEN, format, ap);

    syslog(log_lvl, "%s %s%s", prefix, buff, 
            sz >= LOG_MAX_LEN ? " (...truncated)" : "");
}

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_DEBUG
void _pfs_log_debug(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    _pfs_vlog(LOG_DEBUG, "[debug]", format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_INFO
void _pfs_log_info(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    _pfs_vlog(LOG_INFO, "[info]", format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_WARNING
void _pfs_log_warning(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    _pfs_vlog(LOG_WARNING, "[warning]", format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_ERR
void _pfs_log_err(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    _pfs_vlog(LOG_ERR, "[err]", format, ap);
    va_end(ap);
}
#endif

#if PFS_LOG_LEVEL <= PFS_LOG_LVL_CRIT
void _pfs_log_crit(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    _pfs_vlog(LOG_CRIT, "[crit]", format, ap);
    va_end(ap);
}
#endif

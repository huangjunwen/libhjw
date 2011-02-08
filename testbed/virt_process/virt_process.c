#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <purelibc.h>
#include <errno.h>
#include <stdarg.h>

static sfun _native_syscall;

static int _virt_begin = 0;

static long int _virt_syscall(long int sysno, ...) {
    va_list ap;
    long int a1, a2, a3, a4, a5, a6;
    va_start(ap, sysno);
    a1 = va_arg(ap, long int);
    a2 = va_arg(ap, long int);
    a3 = va_arg(ap, long int);
    a4 = va_arg(ap, long int);
    a5 = va_arg(ap, long int);
    a6 = va_arg(ap, long int);
    va_end(ap);
    /* no fork, no threads */
    if (sysno == SYS_fork)
        return EAGAIN;
    if (sysno == SYS_clone)
        return EPERM;

    return _native_syscall(sysno, a1, a2, a3, a4, a5, a6);
}

static void __attribute__((constructor)) init_virt_process(void) {
    _native_syscall = _pure_start(_virt_syscall, NULL, PUREFLAG_STDALL);
}

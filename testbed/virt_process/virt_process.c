#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <purelibc.h>
#include <errno.h>
#include <stdarg.h>
#include "virt_process.h"

/**********************************
 * virtual system call table
 * ********************************/
#define VSYSCALL_TBL_SZ (512)       // large enough

typedef struct {
    vsyscall_t f;
    int err;                        // set when entry == VSYSCALL_FIX
    long int ret;                   // set when entry == VSYSCALL_FIX
} _vsyscall_entry_t;

static _vsyscall_entry_t _vsyscall_tbl[] = 
    {[0 ... VSYSCALL_TBL_SZ - 1] = {VSYSCALL_DFL, 0, 0}};

/**********************************
 * api
 * ********************************/
int set_vsyscall(long sysno, vsyscall_t f, int err, long ret) {
    _vsyscall_entry_t * e;
    if (sysno < 0 || sysno >= VSYSCALL_TBL_SZ)
        return -1;

    e = _vsyscall_tbl + sysno;
    //TODO: thread safe
    e->ret = ret;
    e->err = err;
    e->f = f;
    return 0;
}

/**********************************
 * virtual system call
 * ********************************/

static sfun _native_syscall;

static long int _dispatch_vsyscall(long int sysno, ...) {
    va_list ap;
    long int a1, a2, a3, a4, a5, a6;
    _vsyscall_entry_t * e;

    va_start(ap, sysno);
    a1 = va_arg(ap, long int);
    a2 = va_arg(ap, long int);
    a3 = va_arg(ap, long int);
    a4 = va_arg(ap, long int);
    a5 = va_arg(ap, long int);
    a6 = va_arg(ap, long int);
    va_end(ap);

    if (sysno < 0 || sysno >= VSYSCALL_TBL_SZ) {
        errno = EINVAL;
        return -1;
    }

    e = _vsyscall_tbl + sysno;
    if (e->f == VSYSCALL_DFL)
        return _native_syscall(sysno, a1, a2, a3, a4, a5, a6);
    else if (e->f == VSYSCALL_FIX) {
        if (e->err)
            errno = e->err;
        return e->ret;
    }
    return e->f(a1, a2, a3, a4, a5, a6);
}

static void __attribute__((constructor)) _init_virt_process(void) {
    _native_syscall = _pure_start(_dispatch_vsyscall, NULL, PUREFLAG_STDALL);
    // test here
    set_vsyscall(SYS_fork, VSYSCALL_FIX, EAGAIN, -1);
    set_vsyscall(SYS_getpid, VSYSCALL_FIX, 0, 0);
}

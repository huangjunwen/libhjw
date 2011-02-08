#ifndef _VIRT_PROCESS_H_
#define _VIRT_PROCESS_H_

typedef long (*vsyscall_t)(long, long, long, long, long, long);

#define VSYSCALL_DFL ((vsyscall_t)0)
#define VSYSCALL_FIX ((vsyscall_t)-1)

extern int set_vsyscall(long sysno, vsyscall_t f, int err, long ret);

#endif

#include <unistd.h>
#include "../log.h"

int main() {
    pfs_log_init();
    pfs_log_debug("debug %d\n", getpid());
    pfs_log_info("info %d\n", getpid());
    pfs_log_warning("warning %d\n", getpid());
    pfs_log_err("err %d\n", getpid());
    pfs_log_crit("crit %d\n", getpid());
    return 0;
}

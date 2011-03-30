#include <assert.h>
#include "partialfs.h"

#define PFS_DENY_PATH(path) r = pfs_deny_path(path, 0); \
        assert(r == 0);

#define PFS_ALLOW_PATH(path) r = pfs_allow_path(path, 0); \
        assert(r == 0);

int main(int argc, char * argv[]) {
    int r;

    pfs_init();
    PFS_DENY_PATH("/");
    PFS_ALLOW_PATH("/bin");
    PFS_ALLOW_PATH("/lib");
    PFS_ALLOW_PATH("/bin/");
    PFS_ALLOW_PATH("/lib/");
    
    return fuse_main(argc, argv, &partialfs_oper, NULL);
}

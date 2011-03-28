#include "partialfs.h"

int main(int argc, char * argv[]) {
    return fuse_main(argc, argv, &partialfs_oper, NULL);
}

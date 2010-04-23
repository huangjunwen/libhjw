#include <unistd.h>
#include <errno.h>
#include "file.hpp"

namespace omq {

bool file_pread(int fd, off_t offset, unsigned char * buff, size_t len) {
    size_t rlen = 0;
    ssize_t ret;
    do {
        if ((ret = pread(fd, (void *)(buff + rlen), len, offset)) == -1) {
            if (errno != EINTR)
                return false;
            ret = 0;
        }
        rlen += ret;
    } while ((len -= ret));
    return true;
}

bool file_pwrite(int fd, off_t offset, const unsigned char * buff, size_t len) {
    ssize_t wlen = 0;
    ssize_t ret;
    do {
        if ((ret = pwrite(fd, (const void *)(buff + wlen), len, offset)) == -1) {
            if (errno != EINTR)
                return false;
            ret = 0;
        }
        wlen += ret;
    } while ((len -= ret));
    return true;
}

bool file_flush(int fd) {
    return fdatasync(fd) == 0;
}


}


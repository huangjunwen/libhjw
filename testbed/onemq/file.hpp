#ifndef _FILE_HPP_
#define _FILE_HPP_

#include <sys/types.h>
#include "serialize.hpp"

namespace omq {

bool file_pread(int fd, off_t offset, unsigned char * buff, size_t len);
bool file_pwrite(int fd, off_t offset, const unsigned char * buff, size_t len);
bool file_flush(int fd);

template <typename T>
bool file_pread(int fd, off_t offset, T * obj) {
    typename buff_t<T>::type buff;
    if (!file_pread(fd, offset, buff, sizeof(buff)))
        return false;
    unserialize(buff, obj);
    return true;
}

template <typename T>
bool file_pwrite(int fd, off_t offset, T obj) {
    typename buff_t<T>::type buff;
    serialize(obj, buff);
    return file_pwrite(fd, offset, buff, sizeof(buff));
}

}

#endif // _FILE_HPP_

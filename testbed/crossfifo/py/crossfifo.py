#!/usr/bin/env python

#
# CupsFS.py: a FUSE filesystem for mounting an LDAP directory in Python
# Need python-fuse bindings, and an LDAP server.
# usage: ./CupsFS.py &lt;mountpoint&gt;
# unmount with fusermount -u &lt;mountpoint&gt;
#

import stat
import errno
import fuse
from time import time

fuse.fuse_python_api = (0, 2)

class Stat(fuse.Stat):
    def __init__(self):
        self.st_mode = stat.S_IFDIR | 0755
        self.st_ino = 0
        self.st_dev = 0
        self.st_nlink = 2
        self.st_uid = 0
        self.st_gid = 0
        self.st_size = 4096
        self.st_atime = 0
        self.st_mtime = 0
        self.st_ctime = 0

class CrossFifoFS(fuse.Fuse):
    def __init__(self, *args, **kw):
        fuse.Fuse.__init__(self, *args, **kw)

    def getattr(self, path):
        st = Stat()
        st.st_atime = int(time())
        st.st_mtime = st.st_atime
        st.st_ctime = st.st_atime
        # return -errno.ENOENT if no entry
        return st

    def readdir(self, path, offset):
        return [fuse.Direntry(x) for x in [ '.', '..' ]]

    def mknod(self, path, mode, dev):
        return 0

    def unlink(self, path):
        return 0

    def read(self, path, size, offset):
        return 'a' * size

    def write(self, path, buf, offset):
        return len(buf)

    def release(self, path, flags):
        return 0

    def open(self, path, flags):
        return 0

    def truncate(self, path, size):
        return 0

    def utime(self, path, times):
        return 0

    def mkdir(self, path, mode):
        return 0

    def rmdir(self, path):
        return 0

    def rename(self, pathfrom, pathto):
        return 0

    def fsync(self, path, isfsyncfile):
        return 0

def main():
    usage="""
        CrossFifoFS: A filesystem to allow 'fifos' across multiple
        machines to work
    """ + fuse.Fuse.fusage

    server = CrossFifoFS(version="%prog " + fuse.__version__,
                    usage=usage, dash_s_do='setsingle')
    server.parse(errex=1)
    server.main()

if __name__ == '__main__':
    main()


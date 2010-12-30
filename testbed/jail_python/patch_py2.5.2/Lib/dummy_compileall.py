
def _run_once():
    import __builtin__
    from __builtin__ import open as _builtin_open
    from cStringIO import StringIO

    class DummyWritableFile(object):
        def __init__(self, filename, mode, _):
            if 'a' in mode:
                raise ValueError("not support appending file")
            self.filename = filename
            self.content = StringIO()

        def write(self, s):
            self.content.write(s)

        def flush(self):
            pass

        def seek(self, offset, whence=0):
            self.content.seek(offset, whence)

        def close(self):
            if self.content.closed:
                return
            s = self.content.getvalue()
            self.content.close()
            import sys
            out = sys.stdout
            out.write(">>>>>>>>>>\n")
            out.write(self.filename + "\n")
            out.write(str(len(s)) + "\n")
            out.write(s)
            out.write("\n")
        
        def __del__(self):
            self.close()

    def open(filename, mode="r", bufsize=-1):
        if "r" in mode or "U" in mode:
            return _builtin_open(filename, mode, bufsize)
        return DummyWritableFile(filename, mode, bufsize)

    __builtin__.__dict__["open"] = open

_run_once()
del _run_once

if __name__ == '__main__':
    import sys
    from compileall import main
    exit_status = int(not main())
    sys.exit(exit_status)

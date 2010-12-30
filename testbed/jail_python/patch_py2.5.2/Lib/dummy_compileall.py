
def _run_once():
    import __builtin__
    from __builtin__ import open as _builtin_open

    class DummyWritableFile(object):
        def __init__(self, filename, mode, bufsize):
            if 'a' in mode:
                raise ValueError("not support appending file")
            self.filename = filename
            self.mode = mode
            self.fptr = 0
            self.content = ''
            self.wrote = False
        
        def write(self, s):
            # at the end, just concat it
            if self.fptr == len(self.content):
                self.content += s
            else:
                b, e = self.fptr, self.fptr + len(s)
                h, t = self.content[:b], self.content[e:]
                self.content = h + s + t
            self.fptr += len(s)

        def flush(self):
            pass

        def seek(self, offset, whence=0):
            if whence == 0:
                if offset < 0:
                    raise ValueError("offset must >= 0")
                self.fptr = offset
            elif whence == 1:
                self.fptr += offset
            elif whence == 2:
                self.fptr = len(self.content) + offset
            else:
                raise ValueError("whence must be in (0, 1, 2)")

        def close(self):
            """
            Simple protocol to describe a file for writing:
            >>>>>>>>>>\\n
            filename\\n
            filelen\\n
            content\\n
            """
            if self.wrote:
                return
            import sys
            out = sys.stdout
            out.write(">>>>>>>>>>\n")
            out.write(self.filename + "\n")
            out.write(str(len(self.content)) + "\n")
            out.write(self.content)
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

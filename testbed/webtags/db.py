import os.path
import re
import sqlite3
import subprocess

def ensure_tables(conn):
    # ensure xref table
    conn.execute("""create table if not exists xref (
        kw text, kind text, lineno integer, filename text, line text)""")
    conn.execute("""create index if not exists xref_kw on xref (kw, kind)""")
    conn.execute("""create index if not exists xref_file on xref (filename, lineno)""")

def load_data(conn, src_dir, extra_ctags_opt=[]):
    src_dir = os.path.normpath(src_dir)
    if not os.path.isdir(src_dir):
        raise ValueError, "%s is not a direcotry" % src_dir

    args = ["ctags", "-x", "--c++-kinds=+lpx", "--c-kinds=+lpx", "--recurse=yes"]
    args.extend(extra_ctags_opt)
    args.append(src_dir)
    output = subprocess.Popen(args, stdout=subprocess.PIPE).stdout

    splitor = re.compile('\s+')
    n = len(src_dir) + 1
    def params():
        for line in output:
            y = splitor.split(line.strip(), 4)
            y[3] = y[3][n:]
            yield y
    conn.execute("delete from xref")
    conn.executemany("insert into xref values (?, ?, ?, ?, ?)", params())
    conn.commit()

conn = sqlite3.connect(":memory:")
ensure_tables(conn)
load_data(conn, "/home/jayven/entry/src/zmq/zeromq2")

from core import DynSql, Env, Nil, NotNil, Unknown

def init(backend):
    backend = backend.lower()
    __import__(backend, globals(), locals(), [], -1)

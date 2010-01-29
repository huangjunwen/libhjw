from base import *
from context import Context
from env import Env

class DynSql(tuple):

    def __new__(cls, dsql, consts=[], env=None):
        return tuple.__new__(cls, (dsql, consts))

    def __init__(self, dsql, consts=[], env=None):
        self.env = env or Env.default
        if not self.env:
            raise ValueError("No Env provided. Perhaps you forget to set the default Env?")
        self.dsql = dsql
        self.consts = consts
        self.fn = self.env.parser(dsql, consts)

    def __repr__(self):
        return "<DynSql %r>" % self.dsql

    def __str__(self):
        return self.dsql

    def __call__(self, d={}, **kw):
        if kw:
            kw.update(d)
            d = kw
        st, sql, args = self.fn(Context(d))
        assert st is NotNil
        return sql, tuple(args)
        
    def specialize(self, d={}, **kw):
        if kw:
            kw.update(d)
            d = kw
        st, dsql, args = self.fn(Context(d, full_eval=False))
        return DynSql(dsql, args, self.env)

from context import Context
from env import Env

class Dynsql(object):

    def __init__(self, dsql, args=[], env=None):
        self.env = env or Env.default
        if not self.env:
            raise ValueError("No Env provided. Perhaps you forget to set the default Env?")
        self.dsql = dsql
        self.args = args
        self.fn = self.env.parser(dsql, args)

    def __repr__(self):
        return "<Dynsql %r>" % self.dsql

    def __dynsql_repr__(self):
        return self.dsql, self.args   

    def __str__(self):
        return self.dsql

    def __call__(self, d):
        st, sql, args = self.fn(Context(d))
        return sql, args
        
    def specialize(self, d):
        st, dsql, args = self.fn(Context(d, full_eval=False))
        return Dynsql(dsql, args, self.env)


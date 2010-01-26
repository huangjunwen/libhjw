
from parser import Parser
from context import Context
from env import Env

class Dynsql(object):

    _parsers = {}

    def __init__(self, env, tmpl, args=[]):
        p = self._parsers.get(env, None)
        if not p:
            p = self._parsers[env] = Parser(env)
        self.tmpl = tmpl
        self.fn = p(tmpl, args)
        self.env = env

    def __call__(self, d):
        st, sql, args = self.fn(Context(d))
        return sql, args
        
    def specialize(self, d):
        st, tmpl, args = self.fn(Context(d, full_eval=False))
        return Dynsql(self.env, tmpl, args)

    def __repr__(self):
        return "<Dynsql %r>" % self.tmpl

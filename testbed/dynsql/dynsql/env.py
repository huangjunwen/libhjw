from base import *
from parser import make_lexer, make_parser

__all__ = ['Env', 'Cntrl']

######################
#        Env         #
######################

class Env(object):

    envs = {}                   # {name: Env}

    default = None              # default env

    def __init__(self, name, placeholder, raw_marker, var_marker, const_marker, 
            cntrl_marker,
            sub_start_marker,
            sub_end_marker):
        loc = locals()
        for n in loc:
            if n == 'self':
                continue
            setattr(self, n, loc[n])
        Env.envs[name] = self

    def as_default(self):
        Env.default = self

    @staticmethod
    def get(name):
        return Env.envs[name]

    @property
    def cntrls(self):
        return Cntrl.cntrls

    @property
    def lexer(self):
        if not hasattr(self, '_lexer'):
            self._lexer = make_lexer(self.raw_marker, self.var_marker, 
                self.const_marker,
                self.cntrl_marker,
                self.sub_start_marker,
                self.sub_end_marker)
        return self._lexer

    @property
    def parser(self):
        if not hasattr(self, '_parser'):
            self._parser = make_parser(self.placeholder, self.raw_marker, 
                self.var_marker,
                self.const_marker,
                self.cntrl_marker,
                self.sub_start_marker,
                self.sub_end_marker,
                self.cntrls,
                self.lexer)
        return self._parser           

######################
#       Cntrls       #
######################
    
class CntrlMeta(type):
    
    def __new__(mcs, name, base, attr):
        t = type.__new__(mcs, name, base, attr)
        if object in base:
            return t
        cntrl_names = attr['names']
        if not cntrl_names:
            raise ValueError("class %s has no cntrl names" % name)
        for n in cntrl_names:
            if Cntrl.cntrls.has_key(n):
                raise ValueError("cntrl name conflict: %s" % n)
            Cntrl.cntrls[n] = t
        return t

class Cntrl(object):
    
    __metaclass__ = CntrlMeta

    cntrls = {}                 # all cntrls

    names = ()

    def __init__(self, param):
        self.param = param

    def __call__(self, val):
        """
        @param val: maybe anythin (include Nil/NotNil/Unknown)
        @return: Nil/NotNil/Unknown
        """
        return NotNil

    def on_succ(self, cntx):
        """
        Call when all sub (except sub-sub) nodes have NotNil result.
        Can modify content in cntx.
        @cntx: dynsqlCntx object
        @return: None
        """

    def on_fail(self, cntx):
        """
        Call when not all sub (except sub-sub) nodes have NotNil result.
        Can modify content in cntx.
        @cntx: dynsqlCntx object
        @return: None
        """

class If(Cntrl):
    
    names = ("if",)

    def __call__(self, val):
        if val is Unknown:
            return Unknown
        if val:
            return NotNil
        return Nil

class Ifn(Cntrl):
    
    names = ("ifn",)

    def __call__(self, val):
        if val is Unknown:
            return Unknown
        if val:
            return Nil 
        return NotNil

class Mutex(Cntrl):
    """Only one sub can get the named mutex"""
    
    names = ("mutex", "m", "mtx")

    def __call__(self, val):
        if val is Unknown:
            return Unknown
        if val:
            return Nil
        return NotNil
    
    def on_succ(self, cntx):
        cntx[self.param] = True

class Succ(Cntrl):
    
    names = ("succ",)

    def on_succ(self, cntx):
        cntx[self.param] = True

    def on_fail(self, cntx):
        cntx[self.param] = False

def Fail(Cntrl):
    
    names = ("fail",)

    def on_fail(self, cntx):
        cntx[self.param] = True

    def on_fail(self, cntx):
        cntx[self.param] = True

class __Nil__(Cntrl):
    
    names = ("nil", "err")

    def __call__(self, val):
        return Nil

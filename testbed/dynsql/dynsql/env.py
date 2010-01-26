from const import *

__all__ = ['Env']

class Env(object):

    cntrls = {}

    def __init__(self, sql_var_marker):
        self.sql_var_marker = sql_var_marker            # %s

    def build_cntrl(self, cntrl_name, param_name):
        cntrl_cls = self.cntrls.get(cntrl_name, None)
        if not cntrl_cls:
            raise ValueError("can't find cntrl named: %s" % cntrl_name)
        return cntrl_cls(param_name)


    
class CntrlMeta(type):
    
    def __new__(mcs, name, base, attr):
        t = type.__new__(mcs, name, base, attr)
        if object in base:
            return t
        cntrl_names = attr['names']
        if not cntrl_names:
            raise ValueError("class %s has no cntrl names" % name)
        for n in cntrl_names:
            if Env.cntrls.has_key(n):
                raise ValueError("cntrl name conflict: %s" % n)
            Env.cntrls[n] = t
        return t


class Cntrl(object):
    
    __metaclass__ = CntrlMeta

    names = ()

    def __init__(self, _):
        pass

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

    def __init__(self, n):
        if not n:
            raise ValueError("A string that length>=1 is expected")
        self.n = n

    def __call__(self, val):
        if val is Unknown:
            return Unknown
        if val:
            return NotNil
        return Nil
    
    def on_succ(self, cntx):
        cntx[self.n] = False

class Err(Cntrl):
    
    names = ("err",)

    def __call__(self, val):
        return Nil


from _dynsql import *

##########################

__all__ = ['DynSql']

class EvalErr(Exception): 
    pass

class SegBase(object):
    
    def __init__(self, raw=None, parent=None, prev=None):

        # link as a tree
        self.parent = parent
        self.child = self.sibling = None 
        if prev is None:                # the first child
            if parent is not None:
                parent.child = self
        else:
            prev.sibling = self

        # sub type init
        self.init(raw)

    #----- sub class methods

    def init(self, raw):
        """
        sub class init
        """
    
    def __call__(self, g, l):
        """
        @param g/l: context for evaluate this seg's value
        @return: (sql_seg, sql_var, next_seg)
            sql_var is a list (maybe empty)
        """
        raise NotImplementedError()


class _Var(object):

    def init(self, raw):
        self.name = raw

    def __call__(self, g, l):
        val = l.get(self.name, None)
        if val is None:
            val = g.get(self.name, None)

        if val is None:
            raise EvalErr()
        
        return self.eval_val(val)


class _Expr(object):

    def init(self, raw):
        self.code = compile(raw, "dynsql", "eval")
    
    def __call__(self, g, l):
        try:
            val = eval(self.code, g, l)
        except:
            raise EvalErr()

        return self.eval_val(val)


class _Invis(object):

    def eval_val(self, val):
        return "", [], self.sibling


class _Raw(object):

    def eval_val(self, val):
        return str(val), [], self.sibling


class _Sql(object):

    def eval_val(self, val):
        if type(val) is tuple:
            if not len(val):
                raise ValueError("empty tuple")
            return "(%s)" % ",".join(["%s",]*len(val)), list(val), self.sibling
        return "%s", [val,], self.sibling

class _Tree(object):
    
    def eval_child(self, g, l):
        sql_segs, sql_vars = [], []
        child = self.child
        while child is not None:
            seg, var, child = child(g, l)
            sql_segs.append(seg)
            sql_vars.extend(var)
        return "".join(sql_segs), sql_vars

##########################

class InvisVarSeg(_Var, _Invis, SegBase): pass

class SqlVarSeg(_Var, _Sql, SegBase): pass

class RawVarSeg(_Var, _Raw, SegBase): pass

class InvisExprSeg(_Expr, _Invis, SegBase): pass

class SqlExprSeg(_Expr, _Sql, SegBase): pass

class RawExprSeg(_Expr, _Sql, SegBase): pass

class OptSeg(_Tree, SegBase):
    
    def __call__(self, g, l):
        try:
            segs, vars = self.eval_child(g, l)
        except EvalErr:
            segs, vars = "", []
        return segs, vars, self.sibling
                
class OrSeg(_Tree, SegBase):

    def __call__(self, g, l):
        try:
            segs, vars = self.eval_child(g, l)
            next = self.sibling
            while type(next) is OrSeg:
                next = next.sibling
        except EvalErr:
            segs, vars = "", []
            if type(self.sibling) is OrSeg:
                next = self.sibling
            else:
                raise EvalErr()
        return segs, vars, next

class PlainSeg(SegBase):
    
    def init(self, raw):
        self.plain = raw

    def __call__(self, g, l):
        return self.plain, [], self.sibling

    
##########################

class DynSql(_Tree):
    
    def __init__(self, tmpl):
        self.tmpl = tmpl
        self.root = None

    def _parse(self):
        tmpl = self.tmpl
        self.root = parent = OptSeg()
        prev = None
        for ev, kind, start, end in dynsqlParser(tmpl):
            if ev == PLAIN:
                prev = PlainSeg(tmpl[start:end], parent, prev)
            elif ev == VAR:
                if kind == '$':
                    t = SqlVarSeg
                elif kind == '?':
                    t = RawVarSeg
                elif kind == '#':
                    t = InvisVarSeg
                prev = t(tmpl[start:end], parent, prev)
            elif ev == EXPR:
                if kind == '$':
                    t = SqlExprSeg
                elif kind == '?':
                    t = RawExprSeg
                elif kind == '#':
                    t = InvisExprSeg
                else:
                    assert 0
                prev = t(tmpl[start:end], parent, prev)
            elif ev == SUB_START:
                if kind == '{':
                    t = OptSeg
                elif kind == '[':
                    t = OrSeg
                else:
                    assert 0
                parent = t(None, parent, prev)
                prev = None
            elif ev == SUB_END:
                prev = parent
                parent = prev.parent
            elif ev == SYNTAX_ERR:
                raise SyntaxError("dynsql syntax error near %r" % tmpl[start:start + 4])
            else:
                assert 0
                
    def __call__(self, cntx, g=None):
        if self.root is None:
            self._parse()
        if g is None:
            g = globals()
        return self.root(g, cntx)[:2]


def test_syntax_err():
    def one(tmpl, cntx={}):
        raised = False
        try:
            DynSql(tmpl)(cntx)
        except SyntaxError, e:
            raised = True
            err = str(e)
        if raised:
            print "%r -> %r -> passed" % (tmpl, err)
        else:
            print "%r, failed" % tmpl

    one("#!abc")        # invalid var
    one("$()")          # empty var
    one(r"'\'\\")       # open string
    one("{ }}")         # bracket mismatch
    one("{{ }")         # bracket mismatch
    one("{]")           # bracket mismatch
    one("[]")           # empty child


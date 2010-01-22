# dynsql lexer

import re
from _lexer import *
from time import time

PLAIN = 1
RAW = 2
VAR = 3
CNTRL = 4
SUB_START = 5
SUB_END = 6

class dynsqlLexer(object):

    delimiter = re.compile(r"""([\?\$#\[\]\"'])""").search

    def __call__(self, tmpl):
        delimiter = self.delimiter
        base = pos = 0
        nest_cnt = 0

        while True:
            m = delimiter(tmpl, pos)
            if not m:
                break

            c = m.group(0)
            start = m.start()

            # if it's string literal
            # just move `pos` forward
            if c in ("'", '"'):
                pos = consume_str(tmpl, start)
                continue

            # is there some plain text to yield
            if start > base:
                s = tmpl[base: start]
                if s and not s.isspace():
                    yield PLAIN, s

            if c in ('$', '?'):
                name, filters, base = extract_var(tmpl, start)   
                if not filters:
                    filters = []
                else:
                    filters = filters.split("|")
                yield (c == '$' and VAR or RAW), (name, filters)
                pos = base
            elif c == '#':
                cntrl, param, base = extract_cntrl(tmpl, start)
                yield CNTRL, (cntrl, param)
                pos = base
            elif c == '[':
                nest_cnt += 1
                yield SUB_START, c
                pos = base = start + 1
            else:
                nest_cnt -= 1
                if nest_cnt < 0:
                    raise SyntaxError("Unbalance brackets at %d" % start)
                yield SUB_END, c
                pos = base = start + 1
    
        if nest_cnt != 0:
            raise SyntaxError("Unbalance brackets at the end")
            
        # remain
        s = tmpl[base:]
        if s and not s.isspace():
            yield PLAIN, s

Defined = object()
Undefined = object()
Unkown = object()

class Cntrl(object):
    
    __cntrl_name__ = None
    __cntrl_name_use_origin__ = True

    @classmethod
    def register(cls, env):
        if cls.__cntrl_name_use_origin__:
            name = cls.__name__
        else:
            name = cls.__cntrl_name__
            if name is None:
                raise TypeError("class %s has not set its __cntrl_name__" % cls.__name__)
        env.setdefault('__cntrl_name_map__', {})[name] = cls.__name__
        env[cls.__name__] = cls

    @staticmethod
    def build_cntrl(cntrl_name, param, env):
        m = env.get("__cntrl_name_map__", None)
        if m is None:
            raise ValueError("can't find cntrl name map")
        real_name = m.get(cntrl_name, None)
        if real_name is None:
            raise ValueError("%s is not a registered cntrl in this env" % cntrl_name)
        return eval(compile(real_name + param, 'dynsql', 'eval'), env)
        
    def on_enter(self, cntx):
        return True

    def on_defined(self, cntx):
        pass

    def on_undefined(self, cntx):
        pass

    def on_unkown(self, cntx):
        pass


class dynsqlParser(object):

    def __init__(self):
        self.lexer = dynsqlLexer()
        self.act_map = {
            PLAIN: self.make_plain,
            RAW: self.make_raw,
            VAR: self.make_var,
            CNTRL: self.make_cntrl,
            SUB_START: self.sub_start,
            SUB_END: self.sub_end,
        }

    def var2placehldr(self, obj):
        if type(obj) is tuple:
            return "[%s]" % ','.join(['%s']*len(obj)), list(obj)
        return "%s", [obj,]

    def parse(self, tmpl, env):
        self.stack = [[], []]
        self.cntrl_stack = [[], []]
        act_map = self.act_map
        for token_type, token in self.lexer(tmpl):
            act_map[token_type](token, env)
        self.sub_end(None, env)
        return self.stack[0][0]

    def make_plain(self, p, env):
        def _plain(cntx):
            return p, []
        self.stack[-1].append(_plain)

    def make_raw(self, p, env):
        name, fs = p
        filters = map(lambda x: eval(compile(x, 'dynsql', 'eval'), env), fs)
        fs.insert(0, name)
        orin = '?(%s)' % '|'.join(fs)
        def _raw(cntx):
            r = reduce(lambda v, f: f(v), filters, cntx.get(name, Undefined))
            if r is Unkown:
                return Unkown, orin
            return r, []
        self.stack[-1].append(_raw)

    def make_var(self, p, env):
        name, fs = p
        filters = map(lambda x: eval(compile(x, 'dynsql', 'eval'), env), fs)
        fs.insert(0, name)
        orin = '$(%s)' % '|'.join(fs)
        def _var(cntx):
            r = reduce(lambda v, f: f(v), filters, cntx.get(name, Undefined))
            if r is Unkown:
                return Unkown, orin
            return self.var2placehldr(r)
        self.stack[-1].append(_var)
        
    def make_cntrl(self, p, env):
        name, param = p
        orin = "#%s%s" % (name, param)
        self.cntrl_stack[-1].append((orin, Cntrl.build_cntrl(name, param, env)))

    def sub_start(self, _, env):
        self.stack.append([])
        self.cntrl_stack.append([])

    def sub_end(self, _, env):
        sub_nodes = self.stack.pop()
        sub_cntrl = self.cntrl_stack.pop()
        if not sub_cntrl:
            orin_head = "["
        else:
            sub_cntrl_orin, sub_cntrl = zip(*sub_cntrl)
            orin_head = "[%s " % ' '.join(sub_cntrl_orin)
        orin_tail = "]"
        def _sub(cntx):
            for cntrl in sub_cntrl:
                if not cntrl.on_enter(cntx):
                    return "", []
            unkown = False
            res = []
            extra = []
            for node in sub_nodes:
                r, e = node(cntx)
                if r is Unkown:
                    unkown = True
                    res.append(e)
                elif r is Undefined:
                    for cntrl in sub_cntrl:
                        cntrl.on_undefined(cntx)
                    return "", []
                else:
                    res.append(r)
                    extra.extend(e)
            if unkown:
                for cntrl in sub_cntrl:
                    cntrl.on_unkown(cntx)
                return "%s%s%s" % (orin_head, "".join(res), orin_tail), []
            for cntrl in sub_cntrl:
                cntrl.on_defined(cntx)
            return "".join(res), extra
        self.stack[-1].append(_sub)


p = dynsqlParser()
f = p.parse("select $a $b", {})

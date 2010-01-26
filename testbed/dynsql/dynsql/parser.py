
import re
from const import *


PLAIN = 1
RAW = 2
VAR = 3
CNTRL = 4
SUB = 5

TOKEN_TYPENAME = {
    PLAIN: "PLAIN",
    RAW: "RAW",
    VAR: "VAR",
    CNTRL: "CNTRL",
    SUB: "SUB",
}

str_liter = r"""(['"])(?:\\.|[^\\])*\%d"""
str_liter_re = re.compile(str_liter % 1).match
def consume_str(s, pos):
    try:
        return str_liter_re(s, pos).end()
    except AttributeError:
        raise SyntaxError("Bad string literal")

var_re = re.compile(r"""\$(\()?(\w+)(?(1)\))""").match
def extract_var(s, pos):
    try:
        m = var_re(s, pos)
        return m.group(2), m.end()
    except AttributeError:
        raise SyntaxError("Bad var literal")
    
raw_re = re.compile(r"""\?(\w+|\((\w+)(?:\s*,\s*(%s))?\))""" % (str_liter % 4)).match
def extract_raw(s, pos):
    try:
        m = raw_re(s, pos)
        g = m.groups()
        return g[1] or g[0], g[2], m.end()
    except AttributeError:
        raise SyntaxError("Bad raw literal")

cntrl_re = re.compile(r"""#([A-z]\w*)\((\w*)\)""").match
def extract_cntrl(s, pos):
    try:
        m = cntrl_re(s, pos)
        g = m.groups()
        return g[0], g[1], m.end()
    except AttributeError:
        raise SyntaxError("Bad cntrl literal")


delim_re = re.compile(r"""([\?\$#\[\]"@])""").search
def lex(tmpl, args):
    base = pos = 0                              # base: base pos of plain, pos: curr search pos
    arg_idx = 0                                 # arg_idx: curr arg index of args
    nest_cnt = 0
    plain = []
    plain_args = []

    while True:
        m = delim_re(tmpl, pos)
        if not m:
            break
        
        c = m.group(0)
        start, end = m.start(), m.end()

        # if it's string literal
        # just move `pos` forward
        if c == '"':
            pos = consume_str(tmpl, start)
            continue
        # if there is a static var
        elif c == '@':
            plain.append(tmpl[base: start])                         # append even empty string
            plain_args.append(args[arg_idx])
            arg_idx += 1
            base = pos = end
            continue

        # remain plain or there is some plain_args
        if start > base or plain_args:
            plain.append(tmpl[base: start])
        # is there some plain to yield
        if plain:
            yield PLAIN, (plain, plain_args)
            plain = []
            plain_args = []

        if c == '$':
            name, base = extract_var(tmpl, start)
            yield VAR, (name,)
        elif c == '?':
            name, default, base = extract_raw(tmpl, start)
            yield RAW, (name, default)
        elif c == '#':
            cntrl_name, name, base = extract_cntrl(tmpl, start)
            yield CNTRL, (cntrl_name, name)
        elif c == '[':
            nest_cnt += 1
            yield SUB, (c,)
            base = end
        else:       # ']'
            nest_cnt -= 1
            if nest_cnt < 0:
                raise SyntaxError("Unbalance brackets at %d" % start)
            yield SUB, (c,)
            base = end

        pos = base
    
    if nest_cnt != 0:
        raise SyntaxError("Unbalance brackets at the end")
            
    # remain
    s = tmpl[base:]
    if s or plain_args:
        plain.append(s)
    if plain:
        yield PLAIN, (plain, plain_args)


class Parser(object):
    
    def __init__(self, env):
        self.act_map = {
            PLAIN: self.make_plain,
            RAW: self.make_raw,
            VAR: self.make_var,
            CNTRL: self.make_cntrl,
            SUB: self.make_sub,
        }
        self.env = env

    def __call__(self, tmpl, args):
        tmpl = tmpl.strip()
        self.stack = [[], []]
        self.cb = [[], []]
        act_map = self.act_map
        env = self.env
        for token_type, token_content in lex(tmpl, args):
            act_map[token_type](env, *token_content)
        self.sub_end(env, True)
        return self.stack[0][0]

    def make_plain(self, env, plain, args):
        orig = "@".join(plain)
        plain = env.sql_var_marker.join(plain)
        def _ret(cntx):
            if cntx.full_eval:
                return NotNil, plain, args
            return NotNil, orig, args
        _ret.token_type = PLAIN
        self.stack[-1].append(_ret)

    def make_raw(self, env, name, default):
        if default is None:
            orig = "?(%s)" % name
            default = Nil
        else:
            orig = "?(%s, %s)" % (name, default)
            default = eval(compile(default, '', 'eval'))
        def _ret(cntx):
            val = cntx[name]                # don't use "val = cntx[name] or default"
            if val is Nil:
                val = default

            if val is Unknown:
                return Unknown, orig, []
            elif val is Nil:
                return Nil, '', []
            return NotNil, str(val), []
        _ret.token_type = RAW
        self.stack[-1].append(_ret)

    def make_var(self, env, name):
        orig = "$(%s)" % name
        marker = env.sql_var_marker
        def _ret(cntx):
            val = cntx[name]
            if val is Unknown:
                return Unknown, orig, []
            elif val is Nil:
                return Nil, '', []
            
            m = cntx.full_eval and marker or '@'
            if type(val) is tuple:
                return NotNil, "(%s)" % ",".join([m]*len(val)), list(val)
            return NotNil, m, [val]
        _ret.token_type = VAR
        self.stack[-1].append(_ret)

    def make_cntrl(self, env, cntrl_name, name):
        orig = "#%s(%s)" % (cntrl_name, name)
        cntrl = env.build_cntrl(cntrl_name, name)
        def _ret(cntx):
            st = cntrl(cntx[name])
            if st is Unknown:
                return st, orig, []
            return st, '', []
        _ret.token_type = CNTRL
        self.stack[-1].append(_ret)
        self.cb[-1].append(cntrl)

    def make_sub(self, env, c):
        if c == '[':
            self.sub_start(env)
        else:
            self.sub_end(env)

    def sub_start(self, env):
        self.stack.append([])
        self.cb.append([])

    def sub_end(self, env, top=False):
        orig_tmpl = top and "%s" or "[%s]"
        sub_nodes = self.stack.pop()
        sub_cb = self.cb.pop()
        def _ret(cntx):
            has_unknown = False
            sqls = []
            args = []
            for node in sub_nodes:
                st, sql, arg = node(cntx)
                if node.token_type != SUB:            # we only pay attention to nodes of this level
                    if st is Nil:
                        for cb in sub_cb:
                            cb.on_fail(cntx)
                        return Nil, '', []
                    elif st is Unknown:
                        has_unknown = True
                sqls.append(sql)
                args.extend(arg)
            
            if has_unknown:
                return Unknown, orig_tmpl % ''.join(sqls), args
            for cb in sub_cb:
                cb.on_succ(cntx)
            return NotNil, ''.join(sqls), args
        _ret.token_type = SUB
        self.stack[-1].append(_ret)


__all__ = TOKEN_TYPENAME.values() + ['TOKEN_TYPENAME', 'Parser']

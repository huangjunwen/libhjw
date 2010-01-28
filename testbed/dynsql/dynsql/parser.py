
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

def make_lexer(raw_marker, var_marker, const_marker, cntrl_marker, 
        sub_start_marker, 
        sub_end_marker):

    str_liter = r"""(['"])(?:\\.|[^\\])*\%d"""
    str_liter_re = re.compile(str_liter % 1).match
    def consume_str(s, pos):
        try:
            return str_liter_re(s, pos).end()
        except AttributeError:
            raise SyntaxError("Bad string literal")

    var_re = re.compile(r"""\%s(\()?(\w+)(?(1)\))""" % var_marker).match
    def extract_var(s, pos):
        try:
            m = var_re(s, pos)
            return m.group(2), m.end()
        except AttributeError:
            raise SyntaxError("Bad var literal")
        
    raw_re = re.compile(r"""\%s(\w+|\((\w+)(?:\s*,\s*(%s))?\))""" % (raw_marker, str_liter % 4)).match
    def extract_raw(s, pos):
        try:
            m = raw_re(s, pos)
            g = m.groups()
            return g[1] or g[0], g[2], m.end()
        except AttributeError:
            raise SyntaxError("Bad raw literal")

    cntrl_re = re.compile(r"""%s([A-z]\w*)\((\w*)\)""" % cntrl_marker).match
    def extract_cntrl(s, pos):
        try:
            m = cntrl_re(s, pos)
            g = m.groups()
            return g[0], g[1], m.end()
        except AttributeError:
            raise SyntaxError("Bad cntrl literal")


    delim_re = re.compile(r"""(['"\%s\%s\%s\%s\%s\%s])""" % (raw_marker, var_marker, 
        const_marker,
        cntrl_marker, 
        sub_start_marker, 
        sub_end_marker)).search
    def lexer(dsql, args):
        base = pos = 0                              # base: base pos of plain, pos: curr search pos
        arg_idx = 0                                 # arg_idx: curr arg index of args
        nest_cnt = 0
        plain = []
        plain_args = []

        while True:
            m = delim_re(dsql, pos)
            if not m:
                break
            
            c = m.group(0)
            start, end = m.start(), m.end()

            # if it's string literal
            # just move `pos` forward
            if c in ("'", '"'):
                pos = consume_str(dsql, start)
                continue
            # if there is a static var
            elif c == const_marker:
                plain.append(dsql[base: start])                         # append even empty string
                plain_args.append(args[arg_idx])
                arg_idx += 1
                base = pos = end
                continue

            # remain plain or there is some plain_args
            if start > base or plain_args:
                plain.append(dsql[base: start])
            # is there some plain to yield
            if plain:
                yield PLAIN, (plain, plain_args)
                plain = []
                plain_args = []

            if c == var_marker:
                name, base = extract_var(dsql, start)
                yield VAR, (name,)
            elif c == raw_marker:
                name, default, base = extract_raw(dsql, start)
                yield RAW, (name, default)
            elif c == cntrl_marker:
                cntrl_name, name, base = extract_cntrl(dsql, start)
                yield CNTRL, (cntrl_name, name)
            elif c == sub_start_marker:
                nest_cnt += 1
                yield SUB, (c,)
                base = end
            else:       # sub_end_marker
                nest_cnt -= 1
                if nest_cnt < 0:
                    raise SyntaxError("Unbalance brackets at %d" % start)
                yield SUB, (c,)
                base = end

            pos = base
        
        if nest_cnt != 0:
            raise SyntaxError("Unbalance brackets at the end")
                
        # remain
        s = dsql[base:]
        if s or plain_args:
            plain.append(s)
        if plain:
            yield PLAIN, (plain, plain_args)

    return lexer


def make_parser(placeholder, raw_marker, var_marker, const_marker, cntrl_marker, 
        sub_start_marker, 
        sub_end_marker,
        cntrls,
        lexer):

    # stack:
    # [
    #   ([sub_nodes, ...], [sub_cntrls, ...]),
    #   ...,
    # ]
    def make_plain(stack, plain, args):
        orig = const_marker.join(plain)
        plain = placeholder.join(plain)
        def _ret(cntx):
            if cntx.full_eval:
                return NotNil, plain, args
            return NotNil, orig, args
        _ret.token_type = PLAIN
        stack[-1][0].append(_ret)

    def make_raw(stack, name, default):
        if default is None:
            orig = "%s(%s)" % (raw_marker, name)
            default = Nil
        else:
            orig = "%s(%s, %s)" % (raw_marker, name, default)
            default = eval(compile(default, '', 'eval'))
        def _ret(cntx):
            # !! don't use "val = cntx[name] or default" since bool(Unknown) is undefined
            val = cntx[name]                
            if val is Nil:
                val = default

            if val is Unknown:
                return Unknown, orig, []
            elif val is Nil:
                return Nil, '', []

            if hasattr(val, '__dynsql_repr__'):
                s, a = val.__dynsql_repr__()
            else:
                s, a = str(val), []
            return NotNil, s, a
        _ret.token_type = RAW
        stack[-1][0].append(_ret)

    def make_var(stack, name):
        orig = "%s(%s)" % (var_marker, name)
        def _ret(cntx):
            val = cntx[name]
            if val is Unknown:
                return Unknown, orig, []
            elif val is Nil:
                return Nil, '', []
            
            m = cntx.full_eval and placeholder or const_marker
            if type(val) is tuple:
                return NotNil, "(%s)" % ",".join([m]*len(val)), list(val)
            return NotNil, m, [val]
        _ret.token_type = VAR
        stack[-1][0].append(_ret)

    def make_cntrl(stack, cntrl_name, param_name):
        orig = "%s%s(%s)" % (cntrl_marker, cntrl_name, param_name)
        cntrl = cntrls[cntrl_name](param_name)
        def _ret(cntx):
            st = cntrl(cntx[param_name])
            if st is Unknown:
                return st, orig, []
            return st, '', []
        _ret.token_type = CNTRL
        stack[-1][0].append(_ret)
        stack[-1][1].append(cntrl)

    def make_sub(stack, c):
        if c == sub_start_marker:
            sub_start(stack)
        else:
            sub_end(stack)

    def sub_start(stack):
        stack.append(([], []))

    def sub_end(stack, top=False):
        if top:
            orig_tmpl = "%s"
        else:
            orig_tmpl = "%s%%s%s" % (sub_start_marker, sub_end_marker)
        sub_nodes, sub_cntrls = stack.pop()

        def _ret(cntx):
            any_unknown = any_nonsub_unkown = False
            sqls, args = [], []
            for node in sub_nodes:
                st, s, a = node(cntx)
                if node.token_type == SUB:
                    if st is Unknown:
                        any_unknown = True
                else:
                    if st is Nil:
                        for c in sub_cntrls:
                            c.on_fail(cntx)
                        return Nil, '', []
                    elif st is Unknown:
                        any_unknown = any_nonsub_unkown = True
                    
                sqls.append(s)
                args.extend(a)

            # only Unknown or NotNil can run to here
            if any_nonsub_unkown:
                return Unknown, orig_tmpl % ''.join(sqls), args

            st = NotNil
            if any_unknown:
                st = Unknown
            
            for c in sub_cntrls:
                c.on_succ(cntx)
            return st, ''.join(sqls), args

        _ret.token_type = SUB
        stack[-1][0].append(_ret)
    
    act_map = {
        PLAIN: make_plain,
        RAW: make_raw,
        VAR: make_var,
        CNTRL: make_cntrl,
        SUB: make_sub,
    }

    def parser(dsql, args):
        dsql = dsql.strip()
        stack = [([], []), ([], [])]
        for token_type, token_content in lexer(dsql, args):
            act_map[token_type](stack, *token_content)
        sub_end(stack, True)
        return stack[0][0][0]

    return parser


__all__ = TOKEN_TYPENAME.values() + ['TOKEN_TYPENAME', 'make_parser', 'make_lexer']

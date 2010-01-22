// vim:fdm=marker:nu:nowrap

#include <Python.h>
#include <string.h>

// consume string literal for s[pos:]
// return the next add of the end of string literal, NULL for failed
static const char * consume_str(const char * s, const char * pos) {
    char q = *pos;
    if (q != '\'' && q != '"')
        return 0;
    const char * p = pos + 1; 
    char c;
    while (( c = *p++ )) {
        if (c == q)
            return p;
        else if (c == '\\')
            ++p;
    }
    return 0;
}

// extract cntrl like #if("t", 1)
// name_start point to 'i'
// name_len == len('if')
// param_start point to '('
// param_len == len('("t", 1)')
// return the next add of the end of cntrl, NULL for failed
static const char * extract_cntrl(const char * s, const char * pos, const char ** name_start, 
        int * name_len, const char ** param_start, int * param_len) {
    const char * p;
    char c;
    int nest_cnt;

    if (*pos != '#')
        return 0;

    p = *name_start = ++pos;
    *name_len = 0;
    *param_start = 0;
    *param_len = 0;
    while (( c = *p++ )) {
        if (isalnum(c) || c == '_')
            continue;
        if (c == '(') {
            if (!(*name_len = p - *name_start - 1))      
                return 0;                                       // '#(...' case
            goto PARAM;
        }
        return 0;                                               // '#x%...' case
    }
    return 0;                                                   // '#x' case

PARAM:
    *param_start = p - 1;                                       // '('
    *param_len = 0;
    nest_cnt = 1;
    while (( c = *p++ )) {
        switch (c) {
        case '(':
            ++nest_cnt;
            break;
        case ')':
            if (--nest_cnt == 0) {
                *param_len = p - *param_start;
                return p;
            }
            break;
        case '\'':
        case '"':
            if (!(p = consume_str(s, p - 1)))
                return 0;
            break;
        default:
            break;
        }
    }
    return 0;
}


// extract var like $x or $(x) or $(x|foo(1, "y")|bar)
// name_start point to 'x'
// name_len == len('x')
// filter_start point to 'f'            may be point to 0 (there is no filter)
// param_len == len('foo(1, "y")|bar')
// return the next add of the end of cntrl, NULL for failed
static const char * extract_var(const char * s, const char * pos, const char ** name_start, 
        int * name_len, const char ** filter_start, int * filter_len) {
    const char * p;
    char c;
    int nest_cnt;
    int enclosed;

    if (*pos != '?' && *pos != '$')
        return 0;
    enclosed = *(++pos) == '(' ? 1 : 0;
    if (enclosed)
        ++pos;
    
    p = *name_start = pos;
    *name_len = 0;
    *filter_start = 0;
    *filter_len = 0;
    while (( c = *p++ )) {
        if (isalnum(c) || c == '_')
            continue;

        if (!enclosed) {
            if (c == ')' || c == '|')
                return 0;                                   // '$x)...' or '$x|' case
            else if (!(*name_len = p - *name_start - 1))
                return 0;                                   // '$%...' case
            return p - 1;                                   // '$x...' case                ok
        }
        else {
            if (c == ')' || c == '|') {
                if (!(*name_len = p - *name_start - 1))
                    return 0;                               // '$()...' or '$(|...' case
                if (c == ')')
                    return p;                               // '$(x)...' case              ok
                else
                    goto FILTER;
            }
            return 0;                                       // '$(asd%...' case 
        }
    }
    if (enclosed)
        return 0;                                           // '$(aljsdf' case
    if (!(*name_len = p - *name_start - 1))
        return 0;                                           // '$' case
    return p - 1;

FILTER:
    *filter_start = p;                                      // next addr of '|'
    *filter_len = 0;
    nest_cnt = 1;
    while (( c = *p++ )) {
        switch (c) {
        case '(':
            ++nest_cnt;
            break;
        case ')':
            if (--nest_cnt == 0) {
                if (!(*filter_len = p - *filter_start - 1)) // '$(x|)' case
                    return 0;
                return p;
            }
            break;
        case '\'':
        case '"':
            if (!(p = consume_str(s, p - 1)))
                return 0;
            break;
        default:
            break;
        }
    }
    return 0;
    
}

static int _check_args(const char * func_name, PyObject * args, const char ** s, int * p) {
    PyObject * str, * pos;
    if (!PyArg_UnpackTuple(args, func_name, 2, 2, &str, &pos))
        return 0;

    if (!PyString_Check(str)) {
        PyErr_Format(PyExc_ValueError, "a string is expected as the first arg");
        return 0;
    }
    if (!PyInt_Check(pos)) {
        PyErr_Format(PyExc_ValueError, "a int is expected as the second arg");
        return 0;
    }

    *s = PyString_AS_STRING(str);
    *p = PyInt_AS_LONG(pos);
    if (*p > PyString_GET_SIZE(str) - 1) {
        PyErr_Format(PyExc_IndexError, "out of range");
        return 0;
    }
    return 1;
}

static PyObject * Py_consume_str(PyObject * self, PyObject * args) {
    const char * s;
    int p;
    if (!_check_args("consume_str", args, &s, &p))
        return 0;
    
    const char * ret = consume_str(s, &s[p]);
    if (!ret) {
        PyErr_Format(PyExc_SyntaxError, "string literal error near %d", p);
        return 0;
    }
    return Py_BuildValue("i", ret - s);
}

static PyObject * Py_extract_cntrl(PyObject * self, PyObject * args) {
    const char * s;
    int p;
    const char * name_start, * param_start;
    int name_len, param_len;
    if (!_check_args("extract_cntrl", args, &s, &p))
        return 0;

    const char * ret = extract_cntrl(s, &s[p], &name_start, &name_len, 
        &param_start, &param_len);
    if (!ret) {
        PyErr_Format(PyExc_SyntaxError, "cntrl syntax error near %d", p);
        return 0;
    }
    return Py_BuildValue("(O,O,i)", PyString_FromStringAndSize(name_start, name_len), 
        PyString_FromStringAndSize(param_start, param_len), ret - s);
}

static PyObject * Py_extract_var(PyObject * self, PyObject * args) {
    const char * s;
    int p;
    const char * name_start, * filter_start;
    int name_len, filter_len;
    if (!_check_args("extract_var", args, &s, &p))
        return 0;

    const char * ret = extract_var(s, &s[p], &name_start, &name_len, 
        &filter_start, &filter_len);
    if (!ret) {
        PyErr_Format(PyExc_SyntaxError, "var syntax error near %d", p);
        return 0;
    }

    PyObject * filter;
    if (filter_start) {
        filter = PyString_FromStringAndSize(filter_start, filter_len);
    }
    else {
        Py_INCREF(Py_None);
        filter = Py_None;
    }
    return Py_BuildValue("(O,O,i)", PyString_FromStringAndSize(name_start, name_len), 
         filter, ret - s);
}

static PyMethodDef
module_functions[] = {
    { "consume_str", Py_consume_str, METH_VARARGS, "consume string literal"},
    { "extract_var", Py_extract_var, METH_VARARGS, "extract dynsql var"},
    { "extract_cntrl", Py_extract_cntrl, METH_VARARGS, "extract dynsql cntrl"},
    { NULL }
};

void
init_lexer(void)
{
    Py_InitModule3("_lexer", module_functions, "some helper function in lexing dynsql");
}

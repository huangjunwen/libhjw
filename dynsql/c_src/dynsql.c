// vim:fdm=marker:nu:nowrap

#include <Python.h>
#include <string.h>

typedef enum {
    UNKNOWN = 0,
    HEAD,                       // head seg
    PLAIN,                      // plain text

    RAW_VAR,                    // raw variable ( `raw` means directly render in the result sql )
    SQL_VAR,                    // variable use in sql ( just a/a list of place holder)
    INVIS_VAR,                  // invisalbe var

    OPT_SEG,                    // optional segment
    OR_SEG                      // or segment
} sqlSegType;

typedef enum {
    NO_ERR = 0,
    MEM_ERR,
    PY_ERR,
    SYNTAX_ERR,
    EMPTY_CHILD
} parseErrCode;

typedef struct sqlSeg {
    struct sqlSeg * parent;     
    struct sqlSeg * first_child;
    struct sqlSeg * sibling;
    sqlSegType type;
    int start;
    int len;
    union {
        PyObject * plain;               // plain string
        struct {
            int is_complex;             // it's a complex expression, not a simple identify
            union {
                PyObject * name;        // simple identify (string object)
                PyObject * code;        // code object (code object)
            };
        } var;
    };
} sqlSeg;


static inline sqlSeg * init_seg(sqlSeg * seg, sqlSegType type, 
        sqlSeg * parent, sqlSeg * prev) {
    memset(seg, 0, sizeof(sqlSeg));
    seg->parent = parent;
    seg->type = type;
    if (prev)
        prev->sibling = seg;
    return seg;
}

static inline sqlSeg * new_seg(sqlSegType type, sqlSeg * parent, 
        sqlSeg * prev) {
    sqlSeg * ret = PyMem_New(sqlSeg, 1);
    if (!ret)
        return 0;
    return init_seg(ret, type, parent, prev);
}

int parse_seg(const char * tmpl, sqlSeg * head_seg, 
        parseErrCode * err_code, int * err_pos) {

    const char *p, *base;
    int len;
    sqlSeg *parent, *last;
    sqlSegType type;
    char c;
    int all_whitespace;

#define ERR_RET(code) { *err_pos = base - tmpl; *err_code = code; return 0; }
#define BEGIN_PLAIN() { base = p; len = 0; all_whitespace = 1; }
#define STORE_PLAIN() if (len && !all_whitespace) { \
    if (!(last = new_seg(PLAIN, parent, last))) ERR_RET(MEM_ERR); \
    last->start = base - tmpl; last->len = len; \
    if (!(last->plain = PyString_FromStringAndSize(base, len))) ERR_RET(PY_ERR); }

    parent = 0;
    last = init_seg(head_seg, HEAD, parent, 0);
    p = tmpl;
    BEGIN_PLAIN();

    // `p` always point to the next char's address
    // `parent` point to last's parent
    // `last` point to the last segment in this level
    while ((c = *p++)) {
        switch (c) {
        case '?': type = RAW_VAR; goto STORE_VAR;
        case '#': type = INVIS_VAR; goto STORE_VAR;
        case '$': type = SQL_VAR; goto STORE_VAR;
        case '[': type = OR_SEG; goto DOWN_TREE;
        case '{': type = OPT_SEG; goto DOWN_TREE;
        case ']': type = OR_SEG; goto UP_TREE;
        case '}': type = OPT_SEG; goto UP_TREE;
        default:
            if (!isspace(c) && isprint(c))
                all_whitespace = 0;
            ++len;
            break;
        }
        continue;

STORE_VAR:
        STORE_PLAIN();

        int is_complex, nest, enclosed, paren_cnt;

        is_complex = 0;
        nest = 0;
        base = p;
        len = 0;
        enclosed = *base == '(';
        paren_cnt = enclosed ? -1 : 0;

        while ((c = *p++)) {
            if (!isprint(c)) {
                ERR_RET(SYNTAX_ERR);
            }
            else if (c == '(') {
                ++nest;
                ++paren_cnt;
            }
            else if (c == ')') {
                if (--nest == 0) {
                    ++len;
                    break;
                }
                if (nest < 0)                               // asymmetric decteced
                    ERR_RET(SYNTAX_ERR);
            }
            else if (!isalnum(c) && c != '_') {
                if (nest == 0) {
                    --p;                                    // backward one char
                    break;
                }
                is_complex = 1;
            }
            ++len;
        }
        if (nest != 0 || !len || (enclosed && len <= 2))    // must be complete and has length
            ERR_RET(SYNTAX_ERR);
        
        if (paren_cnt > 0)
            is_complex = 1;
        
        if (!(last = new_seg(type, parent, last)))
            ERR_RET(MEM_ERR);

        last->start = base - tmpl;
        last->len = len;
        last->var.is_complex = is_complex;
        if (is_complex) {
            char tmp[len + 1];
            strncpy(tmp, base, len);
            tmp[len] = '\0';
            last->var.code = Py_CompileString(tmp, "dynsql", Py_eval_input);
            if (!last->var.code)
                ERR_RET(PY_ERR);
        }
        else {
            if (enclosed)
                last->var.name = PyString_FromStringAndSize(base + 1, len - 2);
            else
                last->var.name = PyString_FromStringAndSize(base, len);
            if (!last->var.name)
                ERR_RET(PY_ERR);
        }

        BEGIN_PLAIN();
        continue;

DOWN_TREE:
        STORE_PLAIN();

        if (!(last = new_seg(type, parent, last)))
            ERR_RET(MEM_ERR);
        last->start = p - tmpl;

        parent = last;                  // down one level
        if (!(last->first_child = new_seg(HEAD, parent, 0)))
            ERR_RET(MEM_ERR);
        last = last->first_child;

        BEGIN_PLAIN();
        continue;

UP_TREE:
        STORE_PLAIN();

        last = parent;
        if (!last || last->type != type)
            ERR_RET(SYNTAX_ERR);
        if (!last->first_child->sibling)
            ERR_RET(EMPTY_CHILD);
        parent = last->parent;
        last->len = p - 1 - tmpl - last->start;

        BEGIN_PLAIN();
        continue;
    }

    STORE_PLAIN();
    if (parent != 0)
        ERR_RET(SYNTAX_ERR);
    *err_code = NO_ERR;
    return 1;   
}

// depth first traverse
void free_seg(sqlSeg * seg) {
    sqlSeg * curr = seg, * next = 0;
    while (curr) {
        switch (curr->type) {
        case PLAIN:
            Py_XDECREF(curr->plain);
        case UNKNOWN:
        case HEAD:
            next = curr->sibling ? curr->sibling : curr->parent;
            break;
        case RAW_VAR:
        case SQL_VAR:
        case INVIS_VAR:
            next = curr->sibling ? curr->sibling : curr->parent;
            if (curr->var.is_complex)
                Py_XDECREF(curr->var.code);
            else
                Py_XDECREF(curr->var.name);
            break;
        case OPT_SEG:
        case OR_SEG:
            next = curr->first_child;
            curr->type = UNKNOWN;
            break;
        }
        PyMem_Del(curr);
        curr = next;
    }
}

typedef struct {
    PyObject_VAR_HEAD
    PyObject * tmpl;        // tmpl string
    int parsed;             // does the tmpl already parsed
    sqlSeg head;            // head seg   
} PyDynSqlObject;


static int dynsql_init(PyDynSqlObject * ds, PyObject * args, PyObject * kw) {
    ds->tmpl = 0;
    if (!PyArg_ParseTuple(args, "|S:__init__", &ds->tmpl))
        return -1;

    if (ds->tmpl != 0)
        Py_INCREF(ds->tmpl);
    else
        ds->tmpl = PyString_FromString("");
    ds->parsed = 0;
    return 0;
}

static void dynsql_dealloc(PyDynSqlObject * ds) {
    if (ds->parsed) {
        free_seg(ds->head.sibling);
    }
    ds->ob_type->tp_free((PyObject *)ds);
}

static PyObject * dynsql_str(PyDynSqlObject * ds) {
    Py_INCREF(ds->tmpl);
    return ds->tmpl;
}

PyTypeObject PyDynSql_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "dynsql",
    sizeof(PyDynSqlObject),
    0,
    (destructor)dynsql_dealloc,     /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    (reprfunc)dynsql_str,           /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "dynsql",                       /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    0,                 /* tp_methods */
    0,                 /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)dynsql_init,          /* tp_init */
    PyType_GenericAlloc,            /* tp_alloc */
    PyType_GenericNew,              /* tp_new */
    0,                              /* tp_free */
};


void initdynsql(void) {
    PyObject * mod;

    // Create the module
    mod = Py_InitModule3("dynsql", NULL, "dynsql c module");
    if (mod == NULL) {
        return;
    }

    if (PyType_Ready(&PyDynSql_Type) < 0) {
        return;
    }

    Py_INCREF(&PyDynSql_Type);
    PyModule_AddObject(mod, "dynsql", (PyObject *)&PyDynSql_Type);
}

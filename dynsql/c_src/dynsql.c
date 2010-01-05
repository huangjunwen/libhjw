// vim:fdm=marker:nu:nowrap

#include <Python.h>
#include <string.h>
#include <stdio.h>
#include "codebase/mem_pool.h"

typedef enum {
    UNKNOWN = 0,
    HEAD,                       // head seg
    PLAIN,                      // plain text

    VAR_TYPE_BEGIN = 10,        // ----
    RAW_VAR,                    // raw variable ( `raw` means directly render in the result sql )
    SQL_VAR,                    // variable use in sql ( just a/a list of place holder)
    INVIS_VAR,                  // invisalbe var

    SEG_TYPE_BEGIN = 20,        // ----
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

static inline sqlSeg * new_seg(memPool * pool, sqlSegType type, 
        sqlSeg * parent, sqlSeg * prev) {
    sqlSeg * ret = (sqlSeg *)mem_pool_get(pool);
    if (!ret)
        return 0;
    return init_seg(ret, type, parent, prev);
}

int dynsql_parse(memPool * pool, const char * tmpl, sqlSeg * head_seg, 
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
    if (!(last = new_seg(pool, PLAIN, parent, last))) ERR_RET(MEM_ERR); \
    last->start = base - tmpl; last->len = len; \
    if (!(last->plain = PyString_FromStringAndSize(base, len))) ERR_RET(PY_ERR); }

    parent = 0;
    last = init_seg(head_seg, HEAD, parent, 0);
    p = tmpl;
    BEGIN_PLAIN();

    // `p` always point to the next char's address
    // `parent` point to last's parent
    // `last` point to the last segment in this level
    while (c = *p++) {
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

        while (c = *p++) {
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
        
        if (!(last = new_seg(pool, type, parent, last)))
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

        if (!(last = new_seg(pool, type, parent, last)))
            ERR_RET(MEM_ERR);
        last->start = p - 1 - tmpl;

        parent = last;                  // down one level
        if (!(last->first_child = new_seg(pool, HEAD, parent, 0)))
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
        last->len = p - tmpl - last->start;

        BEGIN_PLAIN();
        continue;
    }

    STORE_PLAIN();
    if (parent != 0)
        ERR_RET(SYNTAX_ERR);
    *err_code = NO_ERR;
    return 1;   
}


void print_seg(sqlSeg * seg, int indent) {
    while (indent--) printf("\t");
    PyObject * repr = 0;
    const char * type;
    switch (seg->type) {
    case HEAD: type = "HEAD"; break;
    case PLAIN: type = "PLAIN"; repr = PyObject_Repr(seg->plain); break;
    case RAW_VAR: 
        type = "RAW_VAR"; 
        repr = seg->var.is_complex ? seg->var.code : seg->var.name;
        break;
    case SQL_VAR: 
        type = "SQL_VAR"; 
        repr = seg->var.is_complex ? seg->var.code : seg->var.name;
        break;
    case INVIS_VAR: 
        type = "INVIS_VAR"; 
        repr = seg->var.is_complex ? seg->var.code : seg->var.name;
        break;
    case OPT_SEG: type = "OPT_SEG"; break;
    case OR_SEG: type = "OR_SEG"; break;
    }
    printf("%s %d %d |%s|\n", type, seg->start, seg->len, repr ? PyString_AS_STRING(repr) : "");
    Py_XDECREF(repr);
}

void print_segs(sqlSeg * head, int indent) {
    sqlSeg * s = head;
    while (s) {
        print_seg(s, indent);
        if (s->first_child) {
            print_segs(s->first_child, indent+1);
        }
        s = s->sibling;
    }
}

void print_err(const char * tmpl, parseErrCode code, int pos) {
    int len, b;
    char err_buff[21];
    switch (code) {
    case MEM_ERR:
        fprintf(stderr, "not enough memory\n");
        break;
    case SYNTAX_ERR:
        len = strlen(tmpl);
        b = pos <= 10 ? 0 : pos - 10;
        strncpy(err_buff, tmpl + b, sizeof(err_buff) - 1);
        fprintf(stderr, "syntax error near pos %d, \"%s\"\n", pos, err_buff);       
        break;
    case EMPTY_CHILD:
        fprintf(stderr, "empty child\n");
        break;
    }
}

static memPool seg_pool;

int main() {
    Py_Initialize();
    if (!mem_pool_init(&seg_pool, sizeof(sqlSeg), 256))
        return 1;
    
    size_t len = 1024;
    char * tmpl = (char *)malloc(len);
    sqlSeg head;
    int r = getline(&tmpl, &len, stdin);
    tmpl[r - 1] = '\0';
    parseErrCode err_code;
    int err_pos;

    if (!dynsql_parse(&seg_pool, tmpl, &head, &err_code, &err_pos)) {
        print_err(tmpl, err_code, err_pos);
    }
    else {
        print_segs(&head, 0);
    }

    free(tmpl);
    mem_pool_finalize(&seg_pool);
    Py_Finalize();
    return 0;
}

// vim:fdm=marker:nu:nowrap

#include <Python.h>
#include <string.h>

#define _(x) (*(x))
#define CNTX_HEAD int __resume_lineno
#define CNTX_VAR(T, name, cntx) T * name = &((cntx).name)
#define BEGIN(cntx) switch((cntx).__resume_lineno) { default:;
// !!note: do not call YIELD() in an switch statement
#define YIELD(cntx, ret) { (cntx).__resume_lineno = __LINE__ + 1; return (ret); }\
    case __LINE__:;
#define END() }

typedef enum {
    UNKOWN = 0,
    PLAIN,                          // got a plain text
    VAR,                            // got a variable
    EXPR,                           // got a expression
    SUB_START,                      // a sub tree start
    SUB_END                         // a sub tree end
} parserEventType;

typedef struct {
    parserEventType type;
    char kind;                      // '?'/'$'/'#'/'['/'{'
    int start;                      // [start, end)
    int end;
} parserEvent;

typedef struct {
    CNTX_HEAD;
    char c;
    const char * p;                 // current position
    const char * base;              // base position
    int all_whitespace;             // does from base to p are all white spaces

    char * stack;                   // bracket stack
    int stack_sz;
    int stack_capacity;
    int stack_inc;
} parserCntx;

typedef struct {
    PyObject_VAR_HEAD
    PyObject * tmpl;                    // the template
    parserEvent curr;                   // current parser event
    parserCntx cntx;                    // parser contex
} PyDynSqlParser;

static inline char * stack_push(parserCntx * cntx) {
    if (cntx->stack_sz >= cntx->stack_capacity) {
        int new_capacity = cntx->stack_capacity + cntx->stack_inc;
        if (!(cntx->stack = PyMem_Resize(cntx->stack, char, new_capacity)))
            return 0;
        cntx->stack_inc = cntx->stack_capacity;
        cntx->stack_capacity = new_capacity;
    }
    return &cntx->stack[cntx->stack_sz++];
}

static inline char stack_top(parserCntx * cntx) {
    if (!cntx->stack_sz)
        return '\0';
    return cntx->stack[cntx->stack_sz - 1];
}

static inline void stack_pop(parserCntx * cntx) {
    --cntx->stack_sz;
}

#define PARSER_VAR(T, name) CNTX_VAR(T, name, parser->cntx)
#define PARSER_BEGIN() BEGIN(parser->cntx)
#define PARSER_END() END()
#define PARSER_YIELD(T, K, S, E) {          \
            parser->curr.type = (T);        \
            parser->curr.kind = (K);        \
            parser->curr.start = (S);       \
            parser->curr.end = (E);         \
        }                                   \
        YIELD(parser->cntx, 1)

#define BEGIN_PLAIN() {                     \
            _(base) = _(p);                 \
            _(all_whitespace) = 1;          \
        }

#define STORE_PLAIN() if ( _(base) != _(p) && !_(all_whitespace) ) {    \
            PARSER_YIELD(PLAIN, '\0', _(base) - tmpl, _(p) - tmpl);     \
        }
    
// 0 for finish, -1 for failed
int _consume_str(const char * tmpl, const char ** p) {
    char q = *_(p)++;
    int ret = -1;
    char c;
    while (( c = *_(p)++ )) {
        if (c == '\\') {
            ++_(p);
            continue;
        }
        else if (c == q) {
            ret = 0;
            break;
        }
    }
    return ret;
}

// 1 for yielding, 0 for finish, -1 for failed
int _parse(PyDynSqlParser * parser) {
    PARSER_VAR(char, c);
    PARSER_VAR(const char *, p);
    PARSER_VAR(const char *, base);
    PARSER_VAR(int, all_whitespace);

    const char * tmpl = PyString_AS_STRING(parser->tmpl);

    PARSER_BEGIN();

    BEGIN_PLAIN();

    while (( _(c) = *_(p) )) {
        switch ( _(c) ) {
        /*
        case '?':
        case '#':
        case '$':  
            goto STORE_VAR_OR_EXPR;*/
        case '[':
        case '{':
            goto TREE_DOWN;
        case ']':
        case '}':
            goto TREE_UP;
        case '\'':
        case '"':
            if (_consume_str(tmpl, p) < 0)
                return -1;
            continue;
        default:
            if (isprint( _(c) ) && !isspace( _(c) ))
                _(all_whitespace) = 0;
            ++_(p);
            continue;
        }
/*
    STORE_VAR_OR_EXPR:
        STORE_PLAIN();
        BEGIN_PLAIN();
        continue;*/
    TREE_DOWN:
        STORE_PLAIN();

        // push
        *stack_push(&parser->cntx) = _(c);

        // yield
        PARSER_YIELD(SUB_START, _(c), _(p) - tmpl, -1);     // we don't know end so -1

        ++_(p);
        BEGIN_PLAIN();
        continue;
    TREE_UP:
        STORE_PLAIN();
        
        // some check then pop
        if (parser->curr.type == SUB_START)                 // empty child
            return -1;
        if (_(c) - stack_top(&parser->cntx) != 2)           // bracket mismatch
            return -1;
        stack_pop(&parser->cntx);
        
        // yield
        PARSER_YIELD(SUB_END, _(c), -1, _(p) - tmpl);       // we don't know start so -1

        ++_(p);
        BEGIN_PLAIN();
        continue;
    }
    STORE_PLAIN();

    if (parser->cntx.stack_sz)
        return -1;

    PARSER_END();

    return 0;
}



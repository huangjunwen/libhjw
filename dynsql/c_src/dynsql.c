// vim:fdm=marker:nu:nowrap

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
    SYNTAX_ERR,
    EMPTY_CHILD

} parseErrCode;

typedef struct sqlSeg {
    struct sqlSeg * parent;     
    struct sqlSeg * first_child;
    struct sqlSeg * sibling;
    sqlSegType type;
    union {
        struct {
            const char * base;
            int len;
        } plain;                        // plain string
        struct {
            const char * base;          // if "$(abc)" 
            int len;
            int is_complex;             // it's a complex expression, not a simple identify
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
    sqlSeg *parent, *last, *curr;
    sqlSegType type;
    char c;
    int all_whitespace;

#define ERR_RET(code) { *err_pos = base - tmpl; *err_code = code; return 0; }
#define BEGIN_PLAIN() base = p; len = 0; all_whitespace = 1
#define STORE_PLAIN() if (len && !all_whitespace) { \
    if (!(curr = new_seg(pool, PLAIN, parent, last))) ERR_RET(MEM_ERR); \
    curr->plain.base = base; curr->plain.len = len; \
    last = curr; }

    parent = 0;
    last = init_seg(head_seg, HEAD, parent, 0);
    p = tmpl;
    BEGIN_PLAIN();

    // `p` always point to the next char's address
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

        int is_complex, paren_cnt;

        is_complex = 0;
        paren_cnt = 0;
        base = p;
        len = 0;

        while (c = *p++) {
            if (c == '(') {
                ++paren_cnt;
                is_complex = 1;
            }
            else if (c == ')') {
                --paren_cnt;
                if (paren_cnt == 0) {
                    ++len;
                    break;
                }
                if (paren_cnt < 0)              // asymmetric decteced
                    ERR_RET(SYNTAX_ERR);
                
            }
            else if (!isalnum(c) && c != '_') {
                if (paren_cnt == 0) {
                    --p;                        // backward one char
                    break;
                }
                is_complex = 1;
            }
            ++len;
        }
        if (paren_cnt != 0 || !len)                              // the end
            ERR_RET(SYNTAX_ERR);
        
        if (!(curr = new_seg(pool, type, parent, last)))
            ERR_RET(MEM_ERR);

        curr->var.base = base;
        curr->var.len = len;
        curr->var.is_complex = is_complex;
        last = curr;

        BEGIN_PLAIN();
        continue;

DOWN_TREE:
        STORE_PLAIN();

        if (!(curr = new_seg(pool, type, parent, last)))
            ERR_RET(MEM_ERR);

        parent = curr;                  // down one level
        if (!(curr->first_child = new_seg(pool, HEAD, parent, 0)))
            ERR_RET(MEM_ERR);
        last = curr->first_child;

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

        BEGIN_PLAIN();
        continue;
    }

    STORE_PLAIN();
    *err_code = NO_ERR;
    return 1;   
}

void repr(const char * s, int len) {
    char c;
    const char * p = s;
    while (len--) {
        c = *p;
        if (isprint(c))
            printf("%c", c);
        else
            printf("\\x%02x", c);
        ++p;
    }
}

void print_seg(sqlSeg * seg, int indent) {
    while (indent--) printf("\t");
    switch (seg->type) {
    case HEAD: printf("HEAD\n"); break;
    case PLAIN: 
        printf("PLAIN |"); 
        repr(seg->plain.base, seg->plain.len);
        printf("|\n");
        break;
    case RAW_VAR: 
        printf("RAW_VAR |"); 
        repr(seg->plain.base, seg->plain.len);
        printf("|\n");
        break;
    case SQL_VAR: 
        printf("SQL_VAR |"); 
        repr(seg->plain.base, seg->plain.len);
        printf("|\n");
        break;
    case INVIS_VAR: 
        printf("INVIS_VAR |"); 
        repr(seg->plain.base, seg->plain.len);
        printf("|\n");
        break;
    case OPT_SEG: printf("OPT_SEG\n"); break;
    case OR_SEG: printf("OR_SEG\n"); break;
    }
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
    return 0;
}

// vim:fdm=marker:nu:nowrap
#ifndef _DT_H_
#define _DT_H_

#include "../codebase/codebase.h"
#include "mydt_config.h"

typedef real metric;

typedef struct {
    metric x;
    metric y;
    void * attr;
} node;

typedef void (*edgeHandler)(void * extra, const node *, const node *);

// main struct
typedef void* myDt;

// dt struct creation and destruction
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

// dt engine
void dt_begin(myDt dt, edgeHandler handler, void * extra);
void dt_next(myDt dt, metric x, metric y, void * attr);
void dt_end(myDt dt);

#endif

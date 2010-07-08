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

// main struct
typedef void * myDt;

// dt struct creation and destruction
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

// dt params
typedef void (*edgeHandler)(void * param, const node * n1, const node * n2);
void dt_set_edge_handler(myDt, edgeHandler, void *);

typedef void (*trianHandler)(void * param, const node * n1, const node * n2, const node * n3,
    const node * ccc);      // ccc for center of the circumcircle 
void dt_set_trian_handler(myDt, trianHandler, void *);

// dt engine
void dt_begin(myDt dt);
void dt_next(myDt dt, metric x, metric y, void * attr);
void dt_end(myDt dt);

#endif

// vim:fdm=marker:nu:nowrap
#ifndef _DT_H_
#define _DT_H_

#include "types.h"
#include "config.h"

#ifdef INT_METRIC
#error "int metric not support yet"
#else
typedef real metric;
#endif

typedef struct {
    metric x;
    metric y;
    void * attr;
} node;

// node order: from +y -x to -y +x
#define NODE_ORD_CMP(n1, n2) ((n1)->y > (n2)->y || ( (n1)->y == (n2)->y && (n1)->x < (n2)->x ))

typedef void (*edgeHandler)(void *, const node *, const node *);

// main struct
typedef void* myDt;

// dt struct creation and destruction
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

/* three sets of api */
/* 1:
 *      dt_begin/dt_next_sorted/dt_end_sorted
 *      nodes' memory management is outside and nodes' input order is sorted
 *      fastest
 * 2:
 *      dt_begin/dt_next_node/dt_end_node
 *      nodes' memory management is outside
 * 3:
 *      dt_begin/dt_next/dt_end
 *      all managed by dt
 * each run must use only one set of api
 * but different run can use different sets
 */

void dt_begin(myDt dt, edgeHandler handler, void * extra);
void dt_next_sorted(myDt dt, node * pt);
void dt_end_sorted(myDt dt);
void dt_next_node(myDt dt, node * n);
void dt_end_node(myDt dt);
void dt_next(myDt dt, metric x, metric y, void * attr);
void dt_end(myDt dt);

#ifdef COUNT_CALL
// after_break_point count
extern uint32 abp_cnt;
// candidate_circle_event count
extern uint32 cce_cnt;
// handle_site_event count
extern uint32 hse_cnt;
// handle_cirl_event count
extern uint32 hce_cnt;
#endif

#endif

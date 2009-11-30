// vim:fdm=marker:nu:nowrap
#ifndef _DT_H_
#define _DT_H_

#include "config.h"
#include "types.h"

typedef struct {
	real x;
	real y;
} node;

// node order: from +y -x to -y +x
#define NODE_ORD_CMP(n1, n2) ((n1)->y > (n2)->y || ( (n1)->y == (n2)->y && (n1)->x < (n2)->x ))

typedef void (*edgeHandler)(const node *, const node *);

// main struct
typedef void* myDt;

// dt struct creation and destruction
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

/* two sets of api */

void dt_begin(myDt dt, edgeHandler handler);
/* first set
 * dt_begin init a new run
 * dt_next input each node
 * dt_end run all
 */
void dt_next(myDt dt, node * n);
void dt_end(myDt dt);

/* secod set
 * almost the same as the first set
 * but dt_next_sorted must be called under the order specified by NODE_ORD_CMP
 * this set of api is more effecient
 */
void dt_next_sorted(myDt dt, node * pt);
void dt_end_sorted(myDt dt);

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

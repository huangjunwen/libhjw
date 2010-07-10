// vim:fdm=marker:nu:nowrap
#ifndef _DT_H_
#define _DT_H_

#include "../codebase/codebase.h"
#include "mydt_config.h"

/**************************************
 * types
 **************************************/
typedef real metric;

typedef struct {
    metric x;
    metric y;
} node;

typedef void * myDt;

typedef void (*edgeHandler)(void * eh_param, const node * nd1, const node * nd2);

typedef void (*trianHandler)(void * th_param, const node * nd1, const node * nd2, const node * nd3,
    const node * ccc);      // ccc for center of the circumcircle 

/**************************************
 * dt struct creation and destruction
 **************************************/
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

/**************************************
 * dt params
 **************************************/
void dt_set_edge_handler(myDt dt, edgeHandler edge_handler, void * eh_param);
void dt_set_trian_handler(myDt dt, trianHandler trian_handler, void * th_param);

/**************************************
 * dt APIs
 **************************************/
// node order: from +Y to -Y, -X to +X
#define NODE_CMP(n1, n2) ((n1)->y > (n2)->y || ( (n1)->y == (n2)->y && (n1)->x < (n2)->x ))
// sort an array of node pointers in place
void dt_sort_nodes(const node ** nds, uint32_t num);

// 1. simplest API: delaunay triangulate an array of nodes.
//      equal to:
//          dt_sort_nodes(nds, num);
//          dt_run_sorted_nodes(dt, nds, num);
void dt_run_nodes(myDt dt, const node ** nds, uint32_t num);

// 2. delaunay triangulate an array of sorted nodes.
//      equal to:
//          dt_begin_sorted_nodes(dt);
//          for (uint32_t i = 0; i < num; ++i)
//              dt_next_sorted_node(dt, nds[i]);
//          dt_end_sorted_nodes(dt);
void dt_run_sorted_nodes(myDt dt, const node ** nds, uint32_t num);

// 3. this set of API is suitable for some cases that: nodes are already stored in some ordered data structure 
//   (like quadtree) and do not want to copy them to an array.
void dt_begin_sorted_nodes(myDt dt);
void dt_next_sorted_node(myDt dt, const node * nd);
void dt_end_sorted_nodes(myDt dt);

#endif

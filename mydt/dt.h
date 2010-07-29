// vim:fdm=marker:nu:nowrap
#ifndef _DT_H_
#define _DT_H_

#include "typedefs.h"
#include "mem_pool.h"
#include "mydt_config.h"

/**************************************
 * types
 **************************************/
typedef struct {
    float x;
    float y;
} vertex;

typedef void * myDt;

typedef void (*edgeHandler)(void * eh_param, const vertex * v1, const vertex * v2);

typedef void (*trianHandler)(void * th_param, const vertex * v1, const vertex * v2, const vertex * v3,
    const vertex * ccc);      // ccc for center of the circumcircle 

/**************************************
 * dt struct creation and destruction
 **************************************/
boolean_t dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

/**************************************
 * dt params
 **************************************/
void dt_set_edge_handler(myDt dt, edgeHandler edge_handler, void * eh_param);
void dt_set_trian_handler(myDt dt, trianHandler trian_handler, void * th_param);

/**************************************
 * dt APIs
 **************************************/
// delaunay triangulate an array of vertexes.
void dt_run_vertexes(myDt dt, const vertex ** vs, uint32_t num);

#endif

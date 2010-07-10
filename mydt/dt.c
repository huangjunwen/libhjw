// vim:fdm=marker:nu:nowrap

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "dt.h"

/*********************************
 * BST
 *********************************/
#ifdef USE_TREAP
#include "treap.h"
#else
#define NOT_USE_BST
#endif

/*********************************
 * Basic types
 *********************************/

typedef struct wave wave;

typedef node siteEvent;

typedef struct {
    node coord;
    wave * wv;                       // if not null that is the disappear wave when this event occur
} cirlEvent;

struct wave {
    const node * focus;              // the focus point ( the coord of site event )
    cirlEvent * cevent;              // the candidate circle event 
#ifndef NOT_USE_BST
    void * bst_ptr;                    
#endif
    wave * prev;                     // double link list
    wave * next;        
};

#define cevent_init(cev) (memset((cev), 0, sizeof(cirlEvent)))
#define wave_init(wv) (memset((wv), 0, sizeof(wave)))

/*********************************
 * Circle event heap
 *********************************/

typedef struct {
    cirlEvent ** elems;
    uint32_t capacity;
    uint32_t size;
} cevHeap;

INTERNAL boolean ce_heap_init(cevHeap * h) {
    if (!(h->elems = (cirlEvent **)malloc(sizeof(cirlEvent *) * INIT_HEAP_CAPACITY)))
        return 0;
    h->capacity = INIT_HEAP_CAPACITY;
    h->size = 0;
    return 1;
}

INTERNAL void ce_heap_reset(cevHeap * h) {
    h->size = 0;
}

INTERNAL void ce_heap_finalize(cevHeap * h) {
    free(h->elems);
}

INTERNAL boolean ce_heap_push(cevHeap * h, cirlEvent * elem) {
    if (h->size >= h->capacity) {
        void * ne;
        if ( !(ne = realloc(h->elems, sizeof(cirlEvent *) * (h->capacity + h->capacity))) )
            return 0;
        h->elems = (cirlEvent **)ne;
        h->capacity += h->capacity;
    }

    // bottom to top
    uint32_t curr = h->size++;
    uint32_t parent;
    cirlEvent ** elems = h->elems;
    while (curr) {
        parent = (curr - 1) >> 1;
        if (!NODE_CMP(&elem->coord, &elems[parent]->coord))
            break;
        elems[curr] = elems[parent];
        curr = parent;
    }
    elems[curr] = elem;
    return 1;
}

INTERNAL cirlEvent * ce_heap_pop(cevHeap * h) {
    assert(h->size);
    cirlEvent * ret = h->elems[0];
    if (h->size == 1) {
        h->size = 0;
        return ret;
    }
    // last one
    cirlEvent * last = h->elems[--h->size];

    // find postion for last from top to bottom
    uint32_t child;
    uint32_t curr = 0;
    uint32_t last_idx = h->size - 1;
    cirlEvent ** elems = h->elems;
    while ((child = (curr << 1) + 1) <= last_idx){
        if (child + 1 <= last_idx && 
                NODE_CMP(&elems[child + 1]->coord, &elems[child]->coord))
            ++child;
        if (!NODE_CMP(&elems[child]->coord, &last->coord))
            break;
        elems[curr] = elems[child];
        curr = child;
    } 
    elems[curr] = last;
    
    return ret;
}

/*********************************
 * main structure 
 *********************************/

typedef struct {
    memPool ce_pool;                // memory pools
    memPool wv_pool;
    cevHeap ce_heap;                // as a heap
    wave wf_head;                   // the wave front list head
#ifndef NOT_USE_BST
    BST bst;
#endif
    edgeHandler edge_handler;       // edge handler
    void * eh_param;                // param for edge handler
    trianHandler trian_handler;     // triangle handler
    void * th_param;                // param for triangle handler
} myDtImpl;

/*********************************
 * algorithm detail
 *********************************/
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* nodes macro */
#define DELTA(a, b, xory) ((a)->xory - (b)->xory)
#define X_DELTA(a, b) DELTA(a, b, x)
#define Y_DELTA(a, b) DELTA(a, b, y)

#define SUM(a, b, xory) ((a)->xory + (b)->xory)
#define X_SUM(a, b) SUM(a, b, x)
#define Y_SUM(a, b) SUM(a, b, y)

/* circle event macro */
#define LINK_CEVENT(w, e) (w)->cevent = e; (e)->wv = w;
#define UNLINK_CEVENT(w) if ((w)->cevent) {(w)->cevent->wv = 0; (w)->cevent = 0; }

/* wave macro */
#define INIT_WV_SHORTCUT(dt) wave * head = &dt->wf_head
#define HEAD_WV (head)
#define LAST_WV (head->prev)
#define CONNECT_WV(p, n) (p)->next = n; (n)->prev = p

INTERNAL boolean after_break_point(const node * s, const node * l, const node * r) {
    /* after_break_point example graph
     *
     * y
     *     |  l  |
     * ^   \_   _/
     * |     \_/|   |
     * |     |   |r|
     * |    s|    -
     * |-----+--------- sweepline scan from +y to -y
     * |    new site                     
     * +----------------> x
     *
     * return 1 if the site event is occur on the right side of the break point of the two waves
     */
    // case 1, either l or r 's y coord is the same as s
    metric sl_y;
    if (!(sl_y = Y_DELTA(s, l))) {
        return s->x > l->x;
    }
    metric sr_y;
    if (!(sr_y = Y_DELTA(s, r))) {
        return s->x > r->x;
    }

    // case 2, l.y == r.y
    metric lr_y;
    if (!(lr_y = Y_DELTA(l, r))) {
        return s->x > X_SUM(l, r)/2;
    }

    // case 3
    // let Cl be the intersection of line x=s->x and wave l, and yl = 2*Cl.y
    // let Cr be the intersection of line x=s->x and wave r, and yr = 2*Cr.y
    // yl = sweepline + l->y - (s->x - l->x)*(s->x - l->x)/(sweepline - l->y);
    // yr = sweepline + r->y - (s->x - r->x)*(s->x - r->x)/(sweepline - r->y);
    // t = yl - yr
    metric sl_x = X_DELTA(s, l);
    metric sr_x = X_DELTA(s, r);
    metric t = lr_y - sl_x * sl_x / sl_y + sr_x * sr_x / sr_y;

    // two waves have two break points
    // the left break point
    if (lr_y > 0) {
        return t > 0 ? 1 : sr_x > 0;
    }
    // the right break point
    return t < 0 ? 0 : sl_x > 0;
}

INTERNAL cirlEvent * candidate_circle_event(myDtImpl * dt, wave * wv) {
    INIT_WV_SHORTCUT(dt);

    if (wv == HEAD_WV || wv == LAST_WV) {
        return 0;
    }
    const node * a = wv->prev->focus;
    const node * b = wv->focus;
    const node * c = wv->next->focus;

    /* if det
     * | abx cbx | 
     * |         | > 0, then the angle between vector b->a to b->c is less than 180 
     * | aby cby | 
     */
    metric abx = X_DELTA(a, b), aby = Y_DELTA(a, b);
    metric cbx = X_DELTA(c, b), cby = Y_DELTA(c, b);
    metric det = abx * cby - aby * cbx;
    if (det <= 0) {
        return 0;
    }

    cirlEvent * res = (cirlEvent *)mem_pool_get(&dt->ce_pool);
    cevent_init(res);
    node * nd= &res->coord;

    /* get the center of circle of the three points
     * 2*abx*X + 2*aby*Y = abx * X_SUM(a, b) + aby * Y_SUM(a, b)
     * 2*cbx*X + 2*cby*Y = cbx * X_SUM(c, b) + cby * Y_SUM(c, b)
     */
    det *= 2;
    metric r1 = abx * X_SUM(a, b) + aby * Y_SUM(a, b);
    metric r2 = cbx * X_SUM(c, b) + cby * Y_SUM(c, b);
    nd->x = (cby * r1 - aby * r2)/det;
    nd->y = (abx * r2 - cbx * r1)/det;
    
    /* get the bottom point of the circle
     */
    metric xdelta = X_DELTA(nd, a);
    metric ydelta = Y_DELTA(nd, a);
    nd->y -= sqrt(xdelta * xdelta + ydelta * ydelta);
    return res;
}

/*
 * +---+      +---+
 * | l | <--> | r |
 * +---+      +---+
 * if after_break_point(s, l, r) == 1
 *     left_bound = r
 * else
 *     right_bound = l
 */        
INTERNAL void handle_site_event(myDtImpl * dt, const siteEvent * e) {
    INIT_WV_SHORTCUT(dt);

    if (!HEAD_WV->focus) {                          // the first one
        HEAD_WV->focus = e;
        HEAD_WV->next = LAST_WV = HEAD_WV;
#ifndef NOT_USE_BST
        HEAD_WV->bst_ptr = 0;
#endif
        return;
    }
    if (e->y == HEAD_WV->focus->y) {                // the first several one
        wave * w = (wave *)mem_pool_get(&dt->wv_pool);
        wave_init(w);
        w->focus = e;
        CONNECT_WV(LAST_WV, w);
        CONNECT_WV(w, HEAD_WV);
        if (dt->edge_handler)
            dt->edge_handler(dt->eh_param, w->prev->focus, w->focus);
        return;
    }

    wave * left_bound = HEAD_WV;
    wave * right_bound = LAST_WV;
    wave * wv;

#ifndef NOT_USE_BST
    BSTIter iter;    

    BST_ITER_INIT(&dt->bst, &iter);
    while (BST_ITER_NOTNIL(&iter)) {
        wv = (wave *)BST_ITER_DEREF(&iter);
        assert(wv != HEAD_WV);
        if (after_break_point(e, wv->prev->focus, wv->focus)) {
            // search greater part
            left_bound = wv;
            BST_ITER_FORWARD(&iter);
        }
        else {
            // search lesser part
            right_bound = wv->prev;
            BST_ITER_BACKWARD(&iter);
        }
    }
#endif

    wave * curr = left_bound;
    while (curr != right_bound && 
            after_break_point(e, curr->focus, curr->next->focus)) {
        curr = curr->next;
    }
    // curr is the right wave
    
    wave * new_wv = (wave *)mem_pool_get(&dt->wv_pool);
    wave_init(new_wv);
    new_wv->focus = e;
    wave * dup_wv = (wave *)mem_pool_get(&dt->wv_pool);
    wave_init(dup_wv);
    dup_wv->focus = curr->focus;

    // insert in the right place
    // !! must ensure that the order in BST is the same as in wave list
    // left_bound == right_bound when:
    // 1. there is only a head in wave list
    // 2. the last two BST nodes in one iteration point to two adjacent waves
    // in either case new waves should be inserted after curr
    if (left_bound == right_bound || curr != right_bound) {
        // insert after curr
        wv = curr->next;
        CONNECT_WV(curr, new_wv);
        CONNECT_WV(new_wv, dup_wv);
        CONNECT_WV(dup_wv, wv);

    } else {
        // insert before curr
        wv = curr->prev;
        CONNECT_WV(wv, dup_wv);
        CONNECT_WV(dup_wv, new_wv);
        CONNECT_WV(new_wv, curr);

    }
    
    // set false alarm for curr
    UNLINK_CEVENT(curr);

    // recalculate candidate circle event for dup_wv and curr
    cirlEvent * new_cevent;
    cevHeap * heap = &dt->ce_heap;

    if ((new_cevent = candidate_circle_event(dt, curr))) {
        LINK_CEVENT(curr, new_cevent);
        ce_heap_push(heap, new_cevent);
    }
    
    if ((new_cevent = candidate_circle_event(dt, dup_wv))) {
        LINK_CEVENT(dup_wv, new_cevent);
        ce_heap_push(heap, new_cevent);
    }

#ifndef NOT_USE_BST
    if ((rand() & 2) == 0)
        new_wv->bst_ptr = BST_INSERT_AT(&dt->bst, &iter, new_wv);
    else
        new_wv->bst_ptr = 0;
#endif

    // edge handler 
    if (dt->edge_handler)
        dt->edge_handler(dt->eh_param, e, curr->focus);
}

INTERNAL void handle_cirl_event(myDtImpl * dt, cirlEvent * e) {
    wave * wv = e->wv;
    if (!wv)                            // false alarm
        return;
    //INIT_WV_SHORTCUT(dt);
    wave * p = wv->prev;
    wave * n = wv->next;

    // edge handler
    if (dt->edge_handler)
        dt->edge_handler(dt->eh_param, p->focus, n->focus);

    // triangle handler
    if (dt->trian_handler)
        dt->trian_handler(dt->th_param, p->focus, n->focus, wv->focus, &e->coord);

    // remove this wave
    wv->next->prev = p;
    wv->prev->next = n;
    // set false alarms
    UNLINK_CEVENT(p);
    UNLINK_CEVENT(n);
    
    cirlEvent * new_cevent;
    cevHeap * heap = &dt->ce_heap;

    if ((new_cevent = candidate_circle_event(dt, p))) {
        LINK_CEVENT(p, new_cevent);
        ce_heap_push(heap, new_cevent);
    }
    
    if ((new_cevent = candidate_circle_event(dt, n))) {
        LINK_CEVENT(n, new_cevent);
        ce_heap_push(heap, new_cevent);
    }

#ifndef NOT_USE_BST
    BST_DELETE(&dt->bst, wv->bst_ptr);
#endif

    mem_pool_release(&dt->wv_pool, wv);
}

/*********************************
 * APIs
 *********************************/
boolean dt_create(myDt * pdt) {
    myDtImpl * ret = (myDtImpl *)malloc(sizeof(myDtImpl));
    if (!ret)
        return 0;
    if (! (ce_heap_init(&ret->ce_heap) &&
            mem_pool_init(&ret->ce_pool, sizeof(cirlEvent), 128) &&
            mem_pool_init(&ret->wv_pool, sizeof(wave), 128)
#ifndef NOT_USE_BST
            && BST_INIT(&ret->bst)
#endif
            ))
        return 0;
    ret->edge_handler = NULL;
    ret->eh_param = NULL;
    ret->trian_handler = NULL;
    ret->th_param = NULL;

    *pdt = (void *)ret;
    //srand(clock());
    return 1;
}

void dt_destroy(myDt * pdt) {
    myDtImpl * dt = (myDtImpl *)(*pdt);
    ce_heap_finalize(&dt->ce_heap);
    mem_pool_finalize(&dt->ce_pool);
    mem_pool_finalize(&dt->wv_pool);
#ifndef NOT_USE_BST
    BST_FINALIZE(&dt->bst);
#endif
    free(dt);
    *pdt = 0;
}

void dt_set_edge_handler(myDt dt, edgeHandler edge_handler, void * eh_param) {
    ((myDtImpl *)dt)->edge_handler = edge_handler;
    ((myDtImpl *)dt)->eh_param = eh_param;
}

void dt_set_trian_handler(myDt dt, trianHandler trian_handler, void * th_param) {
    ((myDtImpl *)dt)->trian_handler = trian_handler;
    ((myDtImpl *)dt)->th_param = th_param;
}

int node_cmp(const void * elem1, const void * elem2) {
    const node * nd1 = *((const node * const *)elem1);
    const node * nd2 = *((const node * const *)elem2);
    // +Y to -Y
    if (nd1->y > nd2->y)
        return -1;
    else if (nd1->y < nd2->y)
        return 1;
    // -X to +X
    else if (nd1->x < nd2->x)
        return -1;
    else if (nd1->x > nd2->x)
        return 1;
    return 0;
}

void dt_sort_nodes(const node ** nds, uint32_t num) {
    qsort(nds, num, sizeof(const node *), node_cmp);
}

void dt_run_nodes(myDt dt, const node ** nds, uint32_t num) {
    dt_sort_nodes(nds, num);
    dt_run_sorted_nodes(dt, nds, num);
}

void dt_run_sorted_nodes(myDt dt, const node ** nds, uint32_t num) {
    dt_begin_sorted_nodes(dt);
    uint32_t i;
    for (i = 0; i < num; ++i)
        dt_next_sorted_node(dt, nds[i]);
    dt_end_sorted_nodes(dt);
}

void dt_begin_sorted_nodes(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    wave_init(&d->wf_head);
    ce_heap_reset(&d->ce_heap);
    mem_pool_reset(&d->ce_pool);
    mem_pool_reset(&d->wv_pool);
#ifndef NOT_USE_BST
    BST_RESET(&d->bst);
#endif
}

void dt_next_sorted_node(myDt dt, const node * nd) {
    myDtImpl * d = (myDtImpl *)dt;
    cevHeap * heap = &d->ce_heap;
    // run all circle events before the site event
    while (heap->size && NODE_CMP(&heap->elems[0]->coord, nd)) {
        cirlEvent * cev = ce_heap_pop(heap);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }
    handle_site_event(d, nd);
}

void dt_end_sorted_nodes(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    cevHeap * heap = &d->ce_heap;
    // run the reset circle events
    while (heap->size) {
        cirlEvent * cev = ce_heap_pop(heap);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }
}

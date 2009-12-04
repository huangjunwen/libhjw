// vim:fdm=marker:nu:nowrap

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "dt.h"
#include "mem_pool.h"

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
    node * focus;                    // the focus point ( the coord of site event )
    cirlEvent * cevent;              // the circle event 
#ifndef NOT_USE_BST
    void * bst_ptr;                    
#endif
    wave * prev;                     // double link list
    wave * next;        
};

#define cevent_init(cev) (memset((cev), 0, sizeof(cirlEvent)))
#define wave_init(wv) (memset((wv), 0, sizeof(wave)))

/*********************************
 * Site event array
 *********************************/
typedef struct {
    siteEvent ** elems;
    uint32 capacity;
    uint32 size;
} sevArray;

INTERNAL boolean se_array_init(sevArray * a) {
#define INIT_ARRAY_CAPACITY (1024)
    if (!(a->elems = (siteEvent **)malloc(sizeof(siteEvent *) * INIT_ARRAY_CAPACITY)))
        return 0;
    a->capacity = INIT_ARRAY_CAPACITY;
#undef INIT_ARRAY_CAPACITY
    a->size = 0;
    return 1;
}

INTERNAL void se_array_reset(sevArray * a) {
    a->size = 0;
}

INTERNAL void se_array_finalize(sevArray * a) {
    free(a->elems);
}

INTERNAL boolean se_array_push_back(sevArray * a, siteEvent * elem) {
    if (a->size >= a->capacity) {
        void * ne;
        if ( !(ne = realloc(a->elems, sizeof(siteEvent *) * (a->capacity + a->capacity))) )
            return 0;
        a->elems = (siteEvent **)ne;
        a->capacity += a->capacity;
    }
    a->elems[a->size++] = elem;
    return 1;
}

INTERNAL uint32 _partition(siteEvent ** elems, uint32 left, uint32 right) {
    siteEvent * tmp;
#define SWAP(i1, i2) tmp = elems[i1]; elems[i1] = elems[i2]; elems[i2] = tmp
    siteEvent * pivot = elems[left];
    SWAP(left, right);
    uint32 idx = left;
    uint32 i;
    for (i = left; i < right; ++i) {
        if (NODE_ORD_CMP(elems[i], pivot)) {
            SWAP(i, idx);
            ++idx;
        }   
    }   
    SWAP(idx, right);
    return idx;
}

void _qsort(siteEvent ** elems, uint32 left, uint32 right) {
    if (left >= right)
        return;
    uint32 i = _partition(elems, left, right);
    if (i) 
        _qsort(elems, left, i - 1); 
    _qsort(elems, i + 1, right);
}

INTERNAL void se_array_sort(sevArray * a) {
    _qsort(a->elems, 0, a->size - 1); 
}

/*********************************
 * Circle event heap
 *********************************/

typedef struct {
    cirlEvent ** elems;
    uint32 capacity;
    uint32 size;
} cevHeap;

INTERNAL boolean ce_heap_init(cevHeap * h) {
#define INIT_HEAP_CAPACITY (256)
    if (!(h->elems = (cirlEvent **)malloc(sizeof(cirlEvent *) * INIT_HEAP_CAPACITY)))
        return 0;
    h->capacity = INIT_HEAP_CAPACITY;
#undef INIT_HEAP_CAPACITY
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
    uint32 curr = h->size++;
    uint32 parent;
    cirlEvent ** elems = h->elems;
    while (curr) {
        parent = (curr - 1) >> 1;
        if (!NODE_ORD_CMP(&elem->coord, &elems[parent]->coord))
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
    uint32 child;
    uint32 curr = 0;
    uint32 last_idx = h->size - 1;
    cirlEvent ** elems = h->elems;
    while ((child = (curr << 1) + 1) <= last_idx){
        if (child + 1 <= last_idx && 
                NODE_ORD_CMP(&elems[child + 1]->coord, &elems[child]->coord))
            ++child;
        if (!NODE_ORD_CMP(&elems[child]->coord, &last->coord))
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
    memPool nd_pool;
    sevArray se_array;              // as a sorted array
    cevHeap ce_heap;                // as a heap
    wave wf_head;                   // the wave front list head
#ifndef NOT_USE_BST
    BST bst;
#endif
    edgeHandler handler;            // edge handler
    void * extra;                   // extra para for edge handler
} myDtImpl;

boolean dt_create(myDt * pdt) {
    myDtImpl * ret = (myDtImpl *)malloc(sizeof(myDtImpl));
    if (!ret)
        return 0;
    if (! (se_array_init(&ret->se_array) && ce_heap_init(&ret->ce_heap) &&
            mem_pool_init(&ret->ce_pool, sizeof(cirlEvent), 128) &&
            mem_pool_init(&ret->wv_pool, sizeof(wave), 128) &&
            mem_pool_init(&ret->nd_pool, sizeof(node), 256)
#ifndef NOT_USE_BST
            && BST_INIT(&ret->bst)
#endif
            ))
        return 0;
    *pdt = (void *)ret;
    //srand(clock());
    return 1;
}

void dt_destroy(myDt * pdt) {
    myDtImpl * dt = (myDtImpl *)(*pdt);
    se_array_finalize(&dt->se_array);
    ce_heap_finalize(&dt->ce_heap);
    mem_pool_finalize(&dt->ce_pool);
    mem_pool_finalize(&dt->wv_pool);
    mem_pool_finalize(&dt->nd_pool);
#ifndef NOT_USE_BST
    BST_FINALIZE(&dt->bst);
#endif
    free(dt);
    *pdt = 0;
}

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

#ifdef COUNT_CALL
uint32 abp_cnt = 0;
uint32 cce_cnt = 0;
uint32 hse_cnt = 0;
uint32 hce_cnt = 0;
#endif

INTERNAL boolean after_break_point(const node * s, const node * l, const node * r) {
#ifdef COUNT_CALL
    ++abp_cnt;
#endif
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
#ifdef COUNT_CALL
    ++cce_cnt;
#endif
    INIT_WV_SHORTCUT(dt);

    if (wv == HEAD_WV || wv == LAST_WV) {
        return 0;
    }
    node * a = wv->prev->focus;
    node * b = wv->focus;
    node * c = wv->next->focus;

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
INTERNAL void handle_site_event(myDtImpl * dt, siteEvent * e) {
#ifdef COUNT_CALL
    ++hse_cnt;
#endif
    INIT_WV_SHORTCUT(dt);

    if (!HEAD_WV->focus) {                        // the first one
        HEAD_WV->focus = e;
        HEAD_WV->next = LAST_WV = HEAD_WV;
#ifndef NOT_USE_BST
        HEAD_WV->bst_ptr = 0;
#endif
        return;
    }
    if (e->y == HEAD_WV->focus->y) {            // the first several one
        wave * w = (wave *)mem_pool_get(&dt->wv_pool);
        wave_init(w);
        w->focus = e;
        CONNECT_WV(LAST_WV, w);
        CONNECT_WV(w, HEAD_WV);
        dt->handler(dt->extra, w->prev->focus, w->focus);
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
    memPool * pool = &dt->ce_pool;
    cevHeap * heap = &dt->ce_heap;

    if (new_cevent = candidate_circle_event(dt, curr)) {
        LINK_CEVENT(curr, new_cevent);
        ce_heap_push(heap, new_cevent);
    }
    
    if (new_cevent = candidate_circle_event(dt, dup_wv)) {
        LINK_CEVENT(dup_wv, new_cevent);
        ce_heap_push(heap, new_cevent);
    }

#ifndef NOT_USE_BST
    if ((rand() & 2) == 0)
        new_wv->bst_ptr = BST_INSERT_AT(&dt->bst, &iter, new_wv);
    else
        new_wv->bst_ptr = 0;
#endif
    // handler 
    dt->handler(dt->extra, e, curr->focus);
}

INTERNAL void handle_cirl_event(myDtImpl * dt, cirlEvent * e) {
#ifdef COUNT_CALL
    ++hce_cnt;
#endif
    wave * wv = e->wv;
    if (!wv)
        return;
    INIT_WV_SHORTCUT(dt);

    // remove this wave
    wave * p;
    wave * n;
    p = wv->next->prev = wv->prev;
    n = wv->prev->next = wv->next;
    // set false alarms
    UNLINK_CEVENT(p);
    UNLINK_CEVENT(n);
    
    cirlEvent * new_cevent;
    memPool * pool = &dt->ce_pool;
    cevHeap * heap = &dt->ce_heap;

    if (new_cevent = candidate_circle_event(dt, p)) {
        LINK_CEVENT(p, new_cevent);
        ce_heap_push(heap, new_cevent);
    }
    
    if (new_cevent = candidate_circle_event(dt, n)) {
        LINK_CEVENT(n, new_cevent);
        ce_heap_push(heap, new_cevent);
    }

#ifndef NOT_USE_BST
    BST_DELETE(&dt->bst, wv->bst_ptr);
#endif

    mem_pool_release(&dt->wv_pool, wv);
    
    // handler
    dt->handler(dt->extra, p->focus, n->focus);
}

void dt_begin(myDt dt, edgeHandler handler, void * extra) {
    myDtImpl * d = (myDtImpl *)dt;
    d->handler = handler;
    d->extra = extra;
    wave_init(&d->wf_head);
    se_array_reset(&d->se_array);
    ce_heap_reset(&d->ce_heap);
    mem_pool_reset(&d->ce_pool);
    mem_pool_reset(&d->wv_pool);
    mem_pool_reset(&d->nd_pool);
#ifndef NOT_USE_BST
    BST_RESET(&d->bst);
#endif
}

void dt_next_sorted(myDt dt, node * nd) {
    myDtImpl * d = (myDtImpl *)dt;

    cevHeap * heap = &d->ce_heap;
    while (heap->size && NODE_ORD_CMP(&heap->elems[0]->coord, nd)) {
        cirlEvent * cev = ce_heap_pop(heap);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }

    handle_site_event(d, nd);
}

void dt_end_sorted(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    cevHeap * heap = &d->ce_heap;
    while (heap->size) {
        cirlEvent * cev = ce_heap_pop(heap);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }
}

void dt_next_node(myDt dt, node * nd) {
    myDtImpl * d = (myDtImpl *)dt;
    se_array_push_back(&d->se_array, nd);
}

void dt_end_node(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    se_array_sort(&d->se_array);
    uint32 i;
    for (i = 0; i < d->se_array.size; ++i)
        dt_next_sorted(dt, d->se_array.elems[i]);
    dt_end_sorted(dt);
}

void dt_next(myDt dt, metric x, metric y, void * attr) {
    myDtImpl * d = (myDtImpl *)dt;
    node * n = (node *)mem_pool_get(&d->nd_pool);
    n->x = x;
    n->y = y;
    n->attr = attr;
    dt_next_node(dt, n);
}

void dt_end(myDt dt) {
    dt_end_node(dt);
}

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

typedef vertex siteEvent;

typedef struct {
    vertex coord;
    wave * wv;                       // if not null that is the disappear wave when this event occur
} cirlEvent;

struct wave {
    const vertex * focus;            // the focus point ( the coord of site event )
    cirlEvent * cevent;              // the candidate circle event
#ifndef NOT_USE_BST
    void * bst_ptr;
#endif
    wave * prev;                     // double link list
    wave * next;
};

#define cevent_init(cev) (memset((cev), 0, sizeof(cirlEvent)))
#define wave_init(wv) (memset((wv), 0, sizeof(wave)))

// vertex order: from +Y to -Y, -X to +X
#define CMP_VERTEX(v1, v2) ((v1)->y > (v2)->y || ( (v1)->y == (v2)->y && (v1)->x < (v2)->x ))

/*********************************
 * Circle event priority queue
 *********************************/

typedef struct {
    cirlEvent ** elems;
    uint32_t capacity;
    uint32_t size;
} cevPq;

#define CE_PQ_SIZE(pq) ((pq)->size)
#define CE_PQ_HEAD(pq) ((pq)->elems[0])

INTERNAL boolean_t ce_pq_init(cevPq * pq) {
    if (!(pq->elems = (cirlEvent **)malloc(sizeof(cirlEvent *) * INIT_PQ_CAPACITY)))
        return 0;
    pq->capacity = INIT_PQ_CAPACITY;
    pq->size = 0;
    return 1;
}

INTERNAL void ce_pq_reset(cevPq * pq) {
    pq->size = 0;
}

INTERNAL void ce_pq_finalize(cevPq * pq) {
    free(pq->elems);
}

INTERNAL boolean_t ce_pq_enqueue(cevPq * pq, cirlEvent * elem) {
    if (pq->size >= pq->capacity) {
        void * ne;
        if ( !(ne = realloc(pq->elems, sizeof(cirlEvent *) * (pq->capacity + pq->capacity))) )
            return 0;
        pq->elems = (cirlEvent **)ne;
        pq->capacity += pq->capacity;
    }

    // bottom to top
    uint32_t curr = pq->size++;
    uint32_t parent;
    cirlEvent ** elems = pq->elems;
    while (curr) {
        parent = (curr - 1) >> 1;
        if (!CMP_VERTEX(&elem->coord, &elems[parent]->coord))
            break;
        elems[curr] = elems[parent];
        curr = parent;
    }
    elems[curr] = elem;
    return 1;
}

INTERNAL cirlEvent * ce_pq_dequeue(cevPq * pq) {
    assert(pq->size);
    cirlEvent * ret = pq->elems[0];
    if (pq->size == 1) {
        pq->size = 0;
        return ret;
    }
    // last one
    cirlEvent * last = pq->elems[--pq->size];

    // find postion for last from top to bottom
    uint32_t child;
    uint32_t curr = 0;
    uint32_t last_idx = pq->size - 1;
    cirlEvent ** elems = pq->elems;
    while ((child = (curr << 1) + 1) <= last_idx){
        if (child + 1 <= last_idx &&
                CMP_VERTEX(&elems[child + 1]->coord, &elems[child]->coord))
            ++child;
        if (!CMP_VERTEX(&elems[child]->coord, &last->coord))
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
    cevPq ce_pq;                    // as a priority queue
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

/* vertex macro */
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

INTERNAL boolean_t after_break_point(const vertex * s, const vertex * l, const vertex * r) {
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
    if (s->y == l->y)
        return s->x > l->x;
    float sl_y = Y_DELTA(s, l);

    if (s->y == r->y)
        return s->x > r->x;
    float sr_y = Y_DELTA(s, r);

    // case 2, l.y == r.y
    if (l->y == r->y)
        return 2*s->x > X_SUM(l, r);
    float lr_y = Y_DELTA(l, r);

    // case 3
    // let Cl be the intersection of line x=s->x and wave l, and yl = 2*Cl.y
    // let Cr be the intersection of line x=s->x and wave r, and yr = 2*Cr.y
    // yl = sweepline + l->y - (s->x - l->x)*(s->x - l->x)/(sweepline - l->y);
    // yr = sweepline + r->y - (s->x - r->x)*(s->x - r->x)/(sweepline - r->y);
    // t = yl - yr
    float sl_x = X_DELTA(s, l);
    float sr_x = X_DELTA(s, r);
    float t = lr_y - sl_x * sl_x / sl_y + sr_x * sr_x / sr_y;

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
    const vertex * a = wv->prev->focus;
    const vertex * b = wv->focus;
    const vertex * c = wv->next->focus;

    /* if det
     * | abx cbx |
     * |         | > 0, then the angle between vector b->a to b->c is less than 180
     * | aby cby |
     */
    float abx = X_DELTA(a, b), aby = Y_DELTA(a, b);
    float cbx = X_DELTA(c, b), cby = Y_DELTA(c, b);
    float det = abx * cby - aby * cbx;
    if (det <= 0) {
        return 0;
    }

    cirlEvent * res = (cirlEvent *)mem_pool_get(&dt->ce_pool);
    cevent_init(res);
    vertex * v = &res->coord;

    /* get the center of circle of the three points
     * 2*abx*X + 2*aby*Y = abx * X_SUM(a, b) + aby * Y_SUM(a, b)
     * 2*cbx*X + 2*cby*Y = cbx * X_SUM(c, b) + cby * Y_SUM(c, b)
     */
    det *= 2;
    float r1 = abx * X_SUM(a, b) + aby * Y_SUM(a, b);
    float r2 = cbx * X_SUM(c, b) + cby * Y_SUM(c, b);
    v->x = (cby * r1 - aby * r2)/det;
    v->y = (abx * r2 - cbx * r1)/det;

    /* get the bottom point of the circle
     */
    float xdelta = X_DELTA(v, a);
    float ydelta = Y_DELTA(v, a);
    v->y -= sqrt(xdelta * xdelta + ydelta * ydelta);
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
    cevPq * pq = &dt->ce_pq;

    if ((new_cevent = candidate_circle_event(dt, curr))) {
        LINK_CEVENT(curr, new_cevent);
        ce_pq_enqueue(pq, new_cevent);
    }

    if ((new_cevent = candidate_circle_event(dt, dup_wv))) {
        LINK_CEVENT(dup_wv, new_cevent);
        ce_pq_enqueue(pq, new_cevent);
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
        dt->trian_handler(dt->th_param, p->focus, n->focus, wv->focus);

    // remove this wave
    wv->next->prev = p;
    wv->prev->next = n;
    // set false alarms
    UNLINK_CEVENT(p);
    UNLINK_CEVENT(n);

    cirlEvent * new_cevent;
    cevPq * pq = &dt->ce_pq;

    if ((new_cevent = candidate_circle_event(dt, p))) {
        LINK_CEVENT(p, new_cevent);
        ce_pq_enqueue(pq, new_cevent);
    }

    if ((new_cevent = candidate_circle_event(dt, n))) {
        LINK_CEVENT(n, new_cevent);
        ce_pq_enqueue(pq, new_cevent);
    }

#ifndef NOT_USE_BST
    BST_DELETE(&dt->bst, wv->bst_ptr);
#endif

    mem_pool_release(&dt->wv_pool, wv);
}

/*********************************
 * APIs
 *********************************/
boolean_t dt_create(myDt * pdt) {
    myDtImpl * ret = (myDtImpl *)malloc(sizeof(myDtImpl));
    if (!ret)
        return 0;
    if (! (ce_pq_init(&ret->ce_pq) &&
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
    ce_pq_finalize(&dt->ce_pq);
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

int cmp_vertex(const void * elem1, const void * elem2) {
    const vertex * v1 = *((const vertex * const *)elem1);
    const vertex * v2 = *((const vertex * const *)elem2);
    // +Y to -Y
    if (v1->y > v2->y)
        return -1;
    else if (v1->y < v2->y)
        return 1;
    // -X to +X
    else if (v1->x < v2->x)
        return -1;
    else if (v1->x > v2->x)
        return 1;
    return 0;
}


INTERNAL void dt_begin_vertexes(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    wave_init(&d->wf_head);
    ce_pq_reset(&d->ce_pq);
    mem_pool_reset(&d->ce_pool);
    mem_pool_reset(&d->wv_pool);
#ifndef NOT_USE_BST
    BST_RESET(&d->bst);
#endif
}

INTERNAL void dt_next_vertex(myDt dt, const vertex * v) {
    myDtImpl * d = (myDtImpl *)dt;
    cevPq * pq = &d->ce_pq;
    // run all circle events before the site event
    while (CE_PQ_SIZE(pq) && CMP_VERTEX(&CE_PQ_HEAD(pq)->coord, v)) {
        cirlEvent * cev = ce_pq_dequeue(pq);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }
    handle_site_event(d, v);
}

INTERNAL void dt_end_vertexes(myDt dt) {
    myDtImpl * d = (myDtImpl *)dt;
    cevPq * pq = &d->ce_pq;
    // run the reset circle events
    while (CE_PQ_SIZE(pq)) {
        cirlEvent * cev = ce_pq_dequeue(pq);
        handle_cirl_event(d, cev);
        mem_pool_release(&d->ce_pool, cev);
    }
}

void dt_run_vertexes(myDt dt, const vertex ** vs, uint32_t num) {
    qsort(vs, num, sizeof(const vertex *), cmp_vertex);

    dt_begin_vertexes(dt);
    uint32_t i;
    for (i = 0; i < num; ++i)
        dt_next_vertex(dt, vs[i]);
    dt_end_vertexes(dt);
}

// vim:fdm=marker:nu:nowrap

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include "../dt.h"

// options
#define LOOP_NUM (1)

typedef struct output_elem {
    int32_t n1;
    int32_t n2;
    struct output_elem * next;
} output_elem;

typedef struct {
    memPool * pool;
    output_elem * prev;
} output_struct;

void pp_handler(void * extra, const node * p1, const node * p2) {
    int32_t n1 = (uint32_t)p1->attr;
    int32_t n2 = (uint32_t)p2->attr;
    output_struct * st = (output_struct *)extra;
    output_elem * elem = (output_elem *)mem_pool_get(st->pool);
    elem->n1 = n1>n2?n2:n1;
    elem->n2 = n1>n2?n1:n2;
    elem->next = 0;
    st->prev->next = elem;
    st->prev = elem;
}

void tri_handler(void * param, const node * p1, const node * p2, const node * p3, const node * cc) {
    ++(*((int32_t *)param));
}

int main() {

    FILE * fp = fopen("in.node", "r");
    if (!fp) {
        printf("can't open in.node\n");
        return 1;
    }

    // get total number of point
    int32_t total;
    fscanf(fp, "%d 2 0 0\n", &total);
    node * buffer = (node *)malloc(sizeof(node) * total);
    if (!buffer) {
        printf("can't malloc\n");
        return 1;
    }
    // get points
    node * np = buffer;
    int32_t r, n;
    real x, y;
    while (1) {
        r = fscanf(fp, "%d %f %f\n", &n, &x, &y);
        if (r == EOF)
            break;
        np->x = x;
        np->y = y;
        *(int32_t*)(&np->attr) = n;
        ++np;
    }

    myDt dt;
    if (!dt_create(&dt)) {
        printf("Bad\n");
        free(buffer);
        fclose(fp);
        return 1;
    }

    output_struct st;
    output_elem head;
    memPool output_pool;
    mem_pool_init(&output_pool, sizeof(output_elem), 4096);
    st.pool = &output_pool;
    st.prev = &head;
    int32_t tri_cnt;

    dt_set_edge_handler(dt, pp_handler, &st);
    dt_set_trian_handler(dt, tri_handler, &tri_cnt);

    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);

    int32_t i, j;
    for (i = 0; i < LOOP_NUM; ++i) {
        mem_pool_reset(&output_pool);
        tri_cnt = 0;
        dt_begin(dt);
        for (j = 0; j < total; ++j) {
            np = &buffer[j];
            dt_next(dt, np->x, np->y, np->attr);
        }
        dt_end(dt);
    }

    gettimeofday(&tv1, &tz);
    fprintf(stderr, "%d triangles\n", tri_cnt);
    fprintf(stderr, "dt: %ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

    gettimeofday(&tv0, &tz);
    output_elem * e = head.next;
    while (e) {
        printf("%d %d\n", e->n1, e->n2);
        e = e->next;
    }
    gettimeofday(&tv1, &tz);
    fprintf(stderr, "output: %ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

    mem_pool_finalize(&output_pool);
    dt_destroy(&dt);
    free(buffer);
    return 0;
}

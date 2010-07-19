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

typedef struct {
    node coord;
    int32_t num;
} test_node;

typedef struct output_elem {
    int32_t num1;
    int32_t num2;
    struct output_elem * next;
} output_elem;

typedef struct {
    memPool * pool;
    output_elem * prev;
} output_struct;

void pp_handler(void * eh_param, const node * nd1, const node * nd2) {
    output_struct * st = (output_struct *)eh_param;
    output_elem * elem = (output_elem *)mem_pool_get(st->pool);

    int32_t num1 = ((const test_node *)nd1)->num;
    int32_t num2 = ((const test_node *)nd2)->num;
    elem->num1 = num1>num2?num2:num1;
    elem->num2 = num1>num2?num1:num2;
    elem->next = 0;

    st->prev->next = elem;
    st->prev = elem;                // make a link list
}

void tri_handler(void * th_param, const node * nd1, const node * nd2, const node * nd3,
        const node * ccc) {
    ++(*((int32_t *)th_param));
}

int main() {
    int retcode = 0;

    FILE * fp = fopen("in.node", "r");
    if (!fp) {
        printf("can't open in.node\n");
        return 1;
    }

    // get total number of point
    int32_t total;
    fscanf(fp, "%d 2 0 0\n", &total);
    test_node * buffer = (test_node *)malloc(sizeof(test_node) * total);
    if (!buffer) {
        retcode = 1;
        goto NO_NODE_BUFFER;
    }

    const node ** pbuffer = (const node **)malloc(sizeof(node *) * total);
    if (!pbuffer) {
        retcode = 1;
        goto NO_PNODE_BUFFER;
    }

    // get points
    test_node * np = buffer;
    const node ** npp = pbuffer;
    int32_t r, n;
    metric_t x, y;
    while (1) {
        r = fscanf(fp, "%d %f %f\n", &n, &x, &y);
        if (r == EOF)
            break;
        np->coord.x = x;
        np->coord.y = y;
        np->num = n;
        *npp = (node *)np;
        ++np;
        ++npp;
    }

    myDt dt;
    if (!dt_create(&dt)) {
        retcode = 1;
        goto CANT_CREATE_DT;
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

    int32_t i;
    for (i = 0; i < LOOP_NUM; ++i) {
        mem_pool_reset(&output_pool);
        tri_cnt = 0;
        head.next = 0;
        dt_run_nodes(dt, pbuffer, total);
    }

    gettimeofday(&tv1, &tz);
    fprintf(stderr, "%d triangles\n", tri_cnt);
    fprintf(stderr, "dt: %ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

    gettimeofday(&tv0, &tz);
    output_elem * e = head.next;
    while (e) {
        printf("%d %d\n", e->num1, e->num2);
        e = e->next;
    }
    gettimeofday(&tv1, &tz);
    fprintf(stderr, "output: %ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

    mem_pool_finalize(&output_pool);

    dt_destroy(&dt);
CANT_CREATE_DT:
    free(pbuffer);
NO_PNODE_BUFFER:
    free(buffer);
NO_NODE_BUFFER:
    fclose(fp);
    return retcode;
}

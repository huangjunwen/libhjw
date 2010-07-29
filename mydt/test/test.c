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

#define OUTPUT_BUFFER_SZ (50000)

typedef struct {
    vertex coord;
    int32_t num;
} numVertex;

typedef struct {
    int32_t num1;
    int32_t num2;
} outputElem;

outputElem output_buffer[OUTPUT_BUFFER_SZ];
int32_t output_buffer_used = 0;

void print_result(void * eh_param) {
    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);

    int32_t i;
    for (i = 0; i < output_buffer_used; ++i)
        printf("%d %d\n", output_buffer[i].num1, output_buffer[i].num2);

    gettimeofday(&tv1, &tz);
    *(uint32_t *)eh_param += 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l;
}

void edge_handler(void * eh_param, const vertex * nd1, const vertex * nd2) {
    if (output_buffer_used >= OUTPUT_BUFFER_SZ) {
        print_result(eh_param);
        output_buffer_used = 0;
    }

    int32_t num1 = ((const numVertex *)nd1)->num;
    int32_t num2 = ((const numVertex *)nd2)->num;
    output_buffer[output_buffer_used].num1 = num1>num2?num2:num1;
    output_buffer[output_buffer_used].num2  = num1>num2?num1:num2;
    ++output_buffer_used;
}

void tri_handler(void * th_param, const vertex * nd1, const vertex * nd2, const vertex * nd3,
        const vertex * ccc) {
    ++(*((uint32_t *)th_param));
}

int main() {
    int retcode = 0;

    FILE * fp = fopen("in.node", "r");
    if (!fp) {
        fprintf(stderr, "can't open in.node\n");
        return 1;
    }

    // get total number of point
    int32_t total;
    fscanf(fp, "%d 2 0 0\n", &total);
    numVertex * buffer = (numVertex *)malloc(sizeof(numVertex) * total);
    if (!buffer) {
        retcode = 1;
        goto NO_NODE_BUFFER;
    }

    const vertex ** pbuffer = (const vertex **)malloc(sizeof(vertex *) * total);
    if (!pbuffer) {
        retcode = 1;
        goto NO_PNODE_BUFFER;
    }

    // get points
    numVertex * pb = buffer;
    const vertex ** ppb = pbuffer;
    int32_t r, n;
    float x, y;
    while (1) {
        r = fscanf(fp, "%d %f %f\n", &n, &x, &y);
        if (r == EOF)
            break;
        pb->coord.x = x;
        pb->coord.y = y;
        pb->num = n;
        *ppb = (vertex *)pb;
        ++pb;
        ++ppb;
    }

    // ready to calculate
    myDt dt;
    if (!dt_create(&dt)) {
        retcode = 1;
        goto CANT_CREATE_DT;
    }

    uint32_t output_ms = 0;
    uint32_t tri_cnt;

    dt_set_edge_handler(dt, edge_handler, &output_ms);
    dt_set_trian_handler(dt, tri_handler, &tri_cnt);

    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);
    int32_t i;
    for (i = 0; i < LOOP_NUM; ++i) {
        tri_cnt = 0;
        dt_run_vertexes(dt, pbuffer, total);
    }
    print_result(&output_ms);
    gettimeofday(&tv1, &tz);

    // report
    uint32_t total_ms = 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l;
    fprintf(stderr, "%d triangles\n", tri_cnt);
    fprintf(stderr, "dt: %d ms\n", total_ms - output_ms);
    fprintf(stderr, "output: %d ms\n", output_ms);

    // finalize
    dt_destroy(&dt);
CANT_CREATE_DT:
    free(pbuffer);
NO_PNODE_BUFFER:
    free(buffer);
NO_NODE_BUFFER:
    fclose(fp);
    return retcode;
}

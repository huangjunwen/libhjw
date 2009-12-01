// vim:fdm=marker:nu:nowrap

// switches
#define OUTFILE 1
#define SORT_OUTSIDE 1

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Mmsystem.h>
#pragma comment( lib, "winmm.lib" )
#else
#include <sys/time.h>
#endif

// includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "dt.h"
#include "mem_pool.h"


#if OUTFILE
void pp_handler(const node * p1, const node * p2) {
     uint32 n1 = (uint32)p1->attr;
     uint32 n2 = (uint32)p2->attr;
     uint32 min = n1>n2?n2:n1;
     uint32 max = n1>n2?n1:n2;
    printf("%u %u\n", min, max);
    //printf("((%f, %f), (%f, %f))\n", p1->x, p1->y, p2->x, p2->y);
}
#else
void pp_handler(const node * p1, const node * p2) {
    real x_delta = p1->x - p2->x;
    real y_delta = p1->y - p2->y;
    real distance = sqrt(x_delta * x_delta + y_delta * y_delta);
}
#endif

#if SORT_OUTSIDE
int cmp_node_buffer(const void * v1, const void * v2) {
    node * n1 = (node *)v1;
    node * n2 = (node *)v2;
    return NODE_ORD_CMP(n1, n2) ? -1 : 1;
}
#endif

myDt dt;

int main() {
    FILE * fp = fopen("in.txt", "r");
    if (!fp) {
        printf("cant open in.txt\n");
        return 1;
    }
    real x, y;
    int32 r;
    uint32 i, j, num, c;
    node buffer[50000];
    num = 0;
    while (1) {
        r = fscanf(fp, "%d %f %f\n", &c, &x, &y);
        if (r == EOF)
            break;
        buffer[num].x = x;
        buffer[num].y = y;
        *(uint32 *)(&buffer[num].attr) = num;
        ++num;
    }
    fclose(fp);

#if SORT_OUTSIDE
    qsort(buffer, num, sizeof(node), cmp_node_buffer);
#endif
    if (!dt_create(&dt)) {
        printf("Bad\n");
        return 1;
    }

#if OUTFILE
#define LOOP_NUM 1
#else
#define LOOP_NUM 10
#endif
    clock_t c1, c2;
    c1 = clock();

#ifdef WIN32
    DWORD d1, d2;
    d1 = timeGetTime();
#else
    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);
#endif
    for (i = 0; i < LOOP_NUM; ++i) {
        dt_begin(dt, pp_handler);
        for (j = 0; j < num; ++j) {
#if SORT_OUTSIDE
            dt_next_sorted(dt, &buffer[j]);
#else
            dt_next(dt, &buffer[j]);
#endif
        }
#if SORT_OUTSIDE
        dt_end_sorted(dt);
#else
        dt_end(dt);
#endif
    }
    c2 = clock();
#ifdef WIN32
    d2 = timeGetTime();
#else
    gettimeofday(&tv1, &tz);
#endif

    fprintf(stderr, "%d CLOCKS_PER_SEC %d\n", c2 - c1, CLOCKS_PER_SEC);

#ifdef WIN32
    fprintf(stderr, "%d ms\n", d2 - d1);
#else
    fprintf(stderr, "%ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

#endif

#ifdef COUNT_CALL
    fprintf(stderr, "abp_cnt: %u, cce_cnt: %u\n", abp_cnt, cce_cnt);
#endif
    dt_destroy(&dt);
    return 0;
}

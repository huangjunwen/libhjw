// vim:fdm=marker:nu:nowrap

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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../dt.h"
#include "../mem_pool.h"

// options
#define OUTPUT 0
#define LOOP_NUM (100)

void pp_handler(void * extra, const node * p1, const node * p2) {
#if OUTPUT
    int32 n1 = (uint32)p1->attr;
    int32 n2 = (uint32)p2->attr;
    int32 min = n1>n2?n2:n1;
    int32 max = n1>n2?n1:n2;
    printf("%d %d\n", min, max);
#endif
}

int main() {

    FILE * fp = fopen("in.node", "r");
    if (!fp) {
        printf("can't open in.node\n");
        return 1;
    }

    // get total number of point
    int32 total;
    fscanf(fp, "%d 2 0 0\n", &total);
    node buffer[total + 1];
    // get points
    node * np = buffer;
    int32 r, n;
    real x, y;
    while (1) {
        r = fscanf(fp, "%d %f %f\n", &n, &x, &y);
        if (r == EOF)
            break;
        np->x = x;
        np->y = y;
        *(int32*)(&np->attr) = n;
        ++np;
    }

    myDt dt;
    if (!dt_create(&dt)) {
        printf("Bad\n");
        fclose(fp);
        return 1;
    }

#ifdef WIN32
    DWORD d1, d2;
    d1 = timeGetTime();
#else
    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);
#endif

    int32 i, j;
    for (i = 0; i < LOOP_NUM; ++i) {
        dt_begin(dt, pp_handler, 0);
        for (j = 0; j < total; ++j) {
            np = &buffer[j];
            dt_next(dt, np->x, np->y, np->attr);
        }
        dt_end(dt);
    }


#ifdef WIN32
    d2 = timeGetTime();
#else
    gettimeofday(&tv1, &tz);
#endif

#ifdef WIN32
    fprintf(stderr, "%d ms\n", d2 - d1);
#else
    fprintf(stderr, "%ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);
#endif

    dt_destroy(&dt);
    return 0;
}

// vim:fdm=marker:nu:nowrap

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include "dt.h"

// options
#define OUTPUT 1
#define LOOP_NUM (1)

void pp_handler(void * extra, const node * p1, const node * p2) {
#if OUTPUT
    int32_t n1 = (uint32_t)p1->attr;
    int32_t n2 = (uint32_t)p2->attr;
    int32_t min = n1>n2?n2:n1;
    int32_t max = n1>n2?n1:n2;
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

    struct timeval tv0, tv1;
    struct timezone tz;
    gettimeofday(&tv0, &tz);

    int32_t i, j;
    for (i = 0; i < LOOP_NUM; ++i) {
        dt_begin(dt, pp_handler, 0);
        for (j = 0; j < total; ++j) {
            np = &buffer[j];
            dt_next(dt, np->x, np->y, np->attr);
        }
        dt_end(dt);
    }


    gettimeofday(&tv1, &tz);
    fprintf(stderr, "%ld ms\n", 1000l * (tv1.tv_sec - tv0.tv_sec) + (tv1.tv_usec - tv0.tv_usec) / 1000l);

    dt_destroy(&dt);
    free(buffer);
    return 0;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../radix.h"
#include "../partialfs.h"

int main() {
    char * s;
    size_t l, zero;
    int r, vis;
    char * err_msg;
    path_operations_t * ops;

    partialfs_init();

    zero = 0;
    while (1) {
        s = NULL;
        if ((l = getline(&s, &zero, stdin)) < 0)
            break;
        s[--l] = '\0';
        if (!l) {
            free(s);
            break;
        }
        switch (s[0]) {
        case '+': vis = 1; break;
        case '-': vis = 0; break;
        default:
            printf("bad rule, +/- missing\n");
            free(s);
            continue;
        }

        err_msg = NULL;
        r = dcl_path_visibility(s + 1, l - 1, vis, 0, &err_msg);
        if (r < 0)
            printf("bad rule: %s\n", err_msg);
        free(s);
        free(err_msg);
    }

    printf("\nvisibility testing\n");

    while (1) {
        s = NULL;
        if ((l = getline(&s, &zero, stdin)) < 0)
            break;
        s[--l] = '\0';
        if (!l) {
            free(s);
            break;
        }

        if (s[l - 1] == '/') {
            r = get_dpath_visibility(s, l, &ops);
            printf("visibility: %d\n\n", r);
        }
        else {
            r = get_fpath_visibility(s, l, &ops);
            printf("visibility: %d\n\n", r);
        }
        free(s);
    }

    return 0;

}

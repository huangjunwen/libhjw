#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../radix.h"
#include "../partialfs.h"

int main() {
    char * s;
    size_t l, zero;
    int r;

    r = pfs_init();
    assert(r >= 0);

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
        case '+': 
            r = pfs_allow_path(s + 1, l - 1);
            if (r < 0)
                printf("can't allow '%*.s' \n", l - 1, s + 1);
            break;
        case '-': 
            r = pfs_deny_path(s + 1, l - 1);
            if (r < 0)
                printf("can't deny '%*.s' \n", l - 1, s + 1);
            break;
        default:
            printf("bad rule, +/- missing\n");
            break;
        }
        free(s);
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

        r = pfs_get_path_visibility(s, l);
        if (r < 0)
            printf("bad input\n");
        else {
            printf("'%.*s' %s\n",
                l, s, r ? "visible" : "invisible");
        }

        free(s);
    }

    return 0;

}

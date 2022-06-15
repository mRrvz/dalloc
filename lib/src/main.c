#include <stdio.h>
#include "dalloc.h"

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    int *ptr = dalloc(128 * sizeof(int));

    for (int i = 0; i < 128; i++) {
        ptr[i] = rand();
        // printf("%d\n", ptr[i]);
    }

    int *ptr1 = dalloc(50 * sizeof(int));

    for (int i = 0; i < 50; i++) {
        ptr1[i] = rand();
        printf("%d\n", ptr[i]);
    }

    dfree(ptr);

    return 0;
}
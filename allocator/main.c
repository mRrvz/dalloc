#include <stdio.h>
#include "dalloc.h"
#include "list.h"

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    struct list_head list;
    INIT_LIST_HEAD(&list);

    int *ptr = dalloc(128 * sizeof(int));

    for (int i = 0; i < 128; i++) {
        ptr[i] = rand();
        // printf("%d\n", ptr[i]);
    }
    printf("====\n");

#if 0
    int *ptr1 = dalloc(50 * sizeof(int));

    for (int i = 0; i < 50; i++) {
        ptr1[i] = rand();
        printf("%d\n", ptr[i]);
    }
#endif

    dfree(ptr);

    return 0;
}
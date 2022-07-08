#ifndef __CONTAINER_OF__
#define __CONTAINER_OF__

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({          \
    void *__ptr = (void*)(ptr);                       \
    ((type *)(__ptr - offsetof(type, member))); })

#endif
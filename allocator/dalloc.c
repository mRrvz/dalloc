#include "dalloc.h"

#include <stdint.h>
#include <sys/mman.h>
#include <stdbool.h>

#include "list.h"

#ifdef DEBUG
#include <stdio.h>
#endif

typedef struct dalloc_header {
    uint16_t size;
} dalloc_header_t;

typedef struct dalloc_block {
    dalloc_header_t hdr;
    void *mem;
} dalloc_block_t;

typedef struct dalloc_free_blocks {
    dalloc_block_t *block;
    struct list_head node;
} dalloc_free_block_t;

typedef struct dalloc_chunk {
#define DALLOC_CHUNK_SIZE 1024
    void *start;
    void *end;

    dalloc_block_t *maxblock;
    dalloc_free_block_t freeblocks;

    struct list_head node;
} dalloc_chunk_t;

typedef struct dalloc_work {
    bool is_init;
    dalloc_chunk_t *chunk_head;
} dalloc_work_t;

static dalloc_work_t __dalloc = {
    .is_init = false,
    .chunk_head = NULL,
};

#define MMAP(size) \
    mmap(NULL, (size), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)

/*
#define dalloc_get_memblock(mem) \
    (dalloc_block_t *) mem - sizeof(dalloc_block_t)

#define dalloc_block_end(block, size) \
    (char *)((char *)block + sizeof(dalloc_block_t) + size)
    */

#define BLOCK_MEM_OFFSET(block) \
    (char *)block + sizeof(dalloc_block_t)

#define BLOCK_GET_MEM(mem) \
    (dalloc_block_t*)((char *)mem - sizeof(dalloc_block_t))

static inline dalloc_block_t *create_block(uint16_t block_size) {
    dalloc_block_t *block = \
        MMAP(sizeof(dalloc_block_t) + block_size * sizeof(char));

    block->mem = BLOCK_MEM_OFFSET(block);
    block->hdr.size = block_size;

    return block;
}

static dalloc_chunk_t *create_chunk(void) {
    dalloc_chunk_t *chunk = MMAP(sizeof(dalloc_chunk_t));

    chunk->maxblock = create_block(DALLOC_CHUNK_SIZE);
    chunk->freeblocks.block = chunk->maxblock;
    INIT_LIST_HEAD(&(chunk->freeblocks.node));

    chunk->start = chunk->maxblock;
    chunk->end = (char*)chunk->maxblock + DALLOC_CHUNK_SIZE;

    return chunk;
}

__attribute__((unused)) static void free_chunk(dalloc_chunk_t *chunk) {
    munmap(chunk, sizeof(dalloc_chunk_t) + DALLOC_CHUNK_SIZE);
}

#if 0
static void update_maxblock(dalloc_chunk_t **chunk) {}

#define dalloc_remove_block(chunk, b) do { \
    dalloc_free_block_t *curr = (*chunk)->freeblocks; \
    while (curr != NULL && curr->next != b) { \
        curr = curr->next; \
    } \
    if (curr != NULL) \
        curr->next = b->next; \
} while (0)

#define dalloc_add_block_head(chunk, b) do { \
    dalloc_free_block_t *curr = (*chunk)->freeblocks; \
    dalloc_free_block_t *new = dalloc_mmap(sizeof(dalloc_free_block_t)); \
    new->next = curr; \
    new->block = b; \
    (*chunk)->freeblocks = new; \
} while (0)

static dalloc_block_t *get_freeblock(dalloc_chunk_t **chunk, uint16_t size) {
    dalloc_free_block_t *curr = (*chunk)->freeblocks;

#ifdef DEBUG
    printf("\n>>> curr size: %d\n", curr->block->hdr.size);
#endif

    while (curr != NULL && curr->block->hdr.size < size)
        curr = curr->next;

    if (curr == NULL)
        return NULL;

    dalloc_block_t *free_block = curr->block;

    dalloc_block_t *cutted_block = (dalloc_block_t *)dalloc_block_end(free_block, size);
    cutted_block->mem = dalloc_mem_offset(cutted_block);
    cutted_block->hdr.size = free_block->hdr.size - size - 2 * sizeof(dalloc_block_t);

    free_block->hdr.size = size;

    /* Replace this block with a cutted one */
    curr->next = NULL;
    curr->block = cutted_block;
    // dalloc_remove_block(chunk, curr);
    // dalloc_add_block_head(chunk, cutted_block);

    update_maxblock(chunk);

#ifdef DEBUG
    printf("%s:current block size: %d, cutted block size: %d\n", \
        __func__, free_block->hdr.size, cutted_block->hdr.size);
#endif

    return free_block;
}
#endif

static void dalloc_init(void) {
    __dalloc.chunk_head = create_chunk();
    INIT_LIST_HEAD(&(__dalloc.chunk_head->node));
    __dalloc.is_init = true;
}

void *dalloc(size_t cnt) {
    dalloc_chunk_t *curr_chunk = NULL, *tmp = NULL;
    dalloc_block_t *block = NULL;

    if (__dalloc.chunk_head == NULL)
        dalloc_init();

    curr_chunk = __dalloc.chunk_head;
    list_for_each_entry(curr_chunk, &(curr_chunk->node), node) {
        if (curr_chunk->maxblock->hdr.size >= cnt)
            break;
    }

    (void)tmp;
    //tmp = create_chunk();
    //list_add(&(curr_chunk->node), &(tmp->node));
    block = curr_chunk->maxblock;

    //block = get_freeblock(&curr_chunk, cnt);

#ifdef DEBUG
    printf("alloc --->: %s:%d header size, ptrs: %p, %p\n", __func__, block->hdr.size, block, block->mem);
#endif

    return block->mem;
}

void dfree(void *memblock) {
    dalloc_block_t *block = BLOCK_GET_MEM(memblock);

#ifdef DEBUG
    printf("ptrs: %p, %p\n", block, memblock);
    for (int i = 0; i < 128; i++) {
        int *b = (int*)block->mem;
        // printf("%d\n", b[i]);
    }

    printf("free ---> size:%d, block pointer:%p\n", block->hdr.size, block);
#endif

    munmap(block->mem, block->hdr.size);
}
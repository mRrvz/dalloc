#include "dalloc.h"

#define DMALLOC_ALIGN 4

#include <stdint.h>
#include <sys/mman.h>
#include <stdbool.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef struct dalloc_header {
    uint16_t size;
} dalloc_header_t;

typedef struct dalloc_block dalloc_block_t;
typedef struct dalloc_block {
    dalloc_header_t hdr;
    void *mem;
} dalloc_block_t;

typedef struct dalloc_free_blocks dalloc_free_block_t;
typedef struct dalloc_free_blocks {
    dalloc_block_t *block;
    dalloc_free_block_t *next;
} dalloc_free_block_t;

typedef struct dalloc_chunk dalloc_chunk_t;
struct dalloc_chunk {
#define DMALLOC_CHUNK_SIZE 1024

    void *start;
    void *end;

    dalloc_block_t *maxblock;
    dalloc_free_block_t *freeblocks;

    dalloc_chunk_t *next;
    /* dalloc_chunk_t *prev; */
};

bool init = false;
dalloc_chunk_t *chunk_head = NULL;

#define dalloc_mmap(size) \
    mmap(NULL, (size), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)

#define dalloc_get_memblock(mem) \
    (dalloc_block_t *) mem - sizeof(dalloc_block_t)

#define dalloc_block_end(block, size) \
    (char *)((char *)block + sizeof(dalloc_block_t) + size)

#define dalloc_mem_offset(block) \
    block + sizeof(dalloc_block_t)

static inline dalloc_block_t *create_block(uint16_t cnt) {
    dalloc_block_t *block = \
        dalloc_mmap(sizeof(dalloc_block_t) + cnt * sizeof(char));

    block->mem = dalloc_mem_offset(block);
    block->hdr.size = cnt;

    return block;
}

static dalloc_chunk_t *create_chunk(void) {
    dalloc_chunk_t *chunk = dalloc_mmap(sizeof(dalloc_chunk_t));
    chunk->freeblocks = dalloc_mmap(sizeof(dalloc_free_block_t));
    chunk->maxblock = create_block(DMALLOC_CHUNK_SIZE);

    chunk->start = chunk->maxblock;
    chunk->end = chunk->maxblock + DMALLOC_CHUNK_SIZE;

    chunk->freeblocks->block = chunk->maxblock;
    chunk->freeblocks->next = NULL;

    chunk->next = NULL;
    /* chunk->prev = NULL; */

    return chunk;
}

__attribute__((unused)) static void free_chunk(dalloc_chunk_t *chunk) {
    munmap(chunk, sizeof(dalloc_chunk_t) + DMALLOC_CHUNK_SIZE);
}

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

static void dmalloc_init(void) {
    chunk_head = create_chunk();
    init = true;
}

void *dalloc(size_t cnt) {
    if (!init)
        dmalloc_init();

    dalloc_chunk_t *curr_chunk = chunk_head;
    while (curr_chunk->maxblock->hdr.size < cnt) {
        printf("iterate..\n");

        if (curr_chunk->next == NULL)
            curr_chunk->next = create_chunk();

        curr_chunk = curr_chunk->next;
    }

    dalloc_block_t *block = get_freeblock(&curr_chunk, cnt);

#ifdef DEBUG
    printf("%s:%d header size", __func__, block->hdr.size);
#endif

    return block->mem;
}

void dfree(void *mem) {
    dalloc_block_t *block = dalloc_get_memblock(mem);

#ifdef DEBUG
    printf("%s:%p | %p, %zu\n\n", __func__, mem, block, sizeof(dalloc_header_t));
    for (int i = 0; i < 128; i++) {
        int *b = (int*)block->mem;
        printf("%d\n", b[i]);
    }
    printf("suze:%d, block pouinter:%p", block->hdr.size, block);
#endif

    munmap(block->mem, block->hdr.size);
}
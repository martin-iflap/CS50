#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena_allocator.h"

#define ARENA_ALIGNMENT sizeof(void *)



static size_t align_up(size_t n, size_t alignment)
// compute the correct size of the memory chunk so it is aligned well
{
    return (n + alignment - 1) & ~(alignment - 1);
}

static ArenaBlock *arena_new_block(size_t size)
// create new arena block, initialize it and return it
{
    ArenaBlock *block = malloc(sizeof(ArenaBlock));
    block->buffer = malloc(size);
    block->capacity = size;
    block->offset = 0;
    block->next = NULL;
    return block;
}

Arena *arena_create(size_t initial_block_size)
// create new arena object
{
    Arena *arena = malloc(sizeof(Arena));
    arena->block_size = initial_block_size;
    arena->head = arena_new_block(initial_block_size);
    arena->first = arena->head;
    return arena;
}

void *arena_alloc(Arena *arena, size_t size)
// arena malloc equivallent
{
    size_t aligned_offset = align_up(arena->head->offset, ARENA_ALIGNMENT);

    if (aligned_offset + size > arena->head->capacity)
    {
        // current block is full or not big enough
        size_t new_block_size = size > arena->block_size ? size : arena->block_size; // pick the greater one
        ArenaBlock *block = arena_new_block(new_block_size);
        block->next = arena->head;
        arena->head = block;
        aligned_offset = 0;
    }

    void *ptr = arena->head->buffer + aligned_offset;
    arena->head->offset = aligned_offset + size;
    memset(ptr, 0, size); // zero init all the bytes
    return ptr;
}

char *arena_strdup(Arena *arena, const char *str)
// arena strdup equivallent
{
    size_t len = strlen(str) + 1;
    char *copy = arena_alloc(arena, len);
    memcpy(copy, str, len);
    return copy;
}

void arena_destroy(Arena *arena)
// walk the arena blocks and free them one by one
{
    ArenaBlock *block = arena->first;
    while (block)
    {
        ArenaBlock *next = block->next;
        free(block->buffer);
        free(block);
        block = next;
    }
    free(arena);
}

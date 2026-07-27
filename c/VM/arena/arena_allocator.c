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

Arena *arena_create(size_t capacity)
// create the arena with capacity memory bites
{
    Arena *arena = malloc(sizeof(Arena));
    if (!arena)
        return NULL;

    arena->buffer = malloc(capacity);
    if (!arena->buffer)
    {
        free(arena);
        return NULL;
    }

    arena->capacity = capacity;
    arena->offset = 0;
    return arena;
}

void *arena_alloc(Arena *arena, size_t size)
// arena malloc implementation
{
    size_t aligned_offset = align_up(arena->offset, ARENA_ALIGNMENT);

    if (aligned_offset + size > arena->capacity)
    {
        printf("Arena out of memory.\n");
        exit(1);
    }

    void *ptr = arena->buffer + aligned_offset;
    arena->offset = aligned_offset + size;

    memset(ptr, 0, size); // zero all the bites

    return ptr;
}

char *arena_strdup(Arena *arena, const char *str)
// arena strdup implementation
{
    size_t len = strlen(str) + 1;
    char *copy = arena_alloc(arena, len);
    memcpy(copy, str, len);
    return copy;
}

void arena_destroy(Arena *arena)
// free all the arena memory
{
    free(arena->buffer);
    free(arena);
}

// TODO: make the arena grow in the future

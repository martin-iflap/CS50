#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>


typedef struct
{
    unsigned char *buffer;
    size_t capacity;
    size_t offset;
} Arena;


Arena *arena_create(size_t capacity);
void *arena_alloc(Arena *arena, size_t size);
char *arena_strdup(Arena *arena, const char *str);
void arena_destroy(Arena *arena);


#endif
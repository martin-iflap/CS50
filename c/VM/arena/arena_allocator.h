#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>


typedef struct ArenaBlock ArenaBlock;
struct ArenaBlock
{
    unsigned char *buffer;
    size_t capacity;
    size_t offset; // offset from buffer start
    ArenaBlock *next;
};

typedef struct
{
    ArenaBlock *head;    // block currently being allocated from
    ArenaBlock *first;   // kept only so arena_destroy can walk and free all blocks
    size_t block_size;   // size used for each new block
} Arena;

Arena *arena_create(size_t initial_block_size);
void *arena_alloc(Arena *arena, size_t size);
char *arena_strdup(Arena *arena, const char *str);
void arena_destroy(Arena *arena);


#endif
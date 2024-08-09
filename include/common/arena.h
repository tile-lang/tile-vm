#ifndef ARENA_H
#define ARENA_H

typedef unsigned char   u8_t;
typedef unsigned short  u16_t;
typedef unsigned int    u32_t;
typedef unsigned long   u64_t;
typedef signed   char   i8_t;
typedef signed   short  i16_t;
typedef signed   int    i32_t;
typedef signed   long   i64_t;

typedef struct arena_struct {
    u64_t size;
    u64_t capacity;
    u8_t* memory;

} arena_t;

arena_t arena_init(u64_t size);
void* arena_alloc(arena_t* arena, u64_t size);
void* arena_realloc(arena_t* arena, u64_t size);
void arena_reset(arena_t* arena);
void arena_destroy(arena_t* arena);


#ifdef ARENA_IMPLEMENTATION
#undef ARENA_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

arena_t arena_init(u64_t size) {
    arena_t arena = {
        .capacity = size,
        .size = 0,
        .memory = (u8_t*)malloc(size)
    };
    return arena;
}

void* arena_alloc(arena_t* arena, u64_t size) {
    void* ptr = NULL;
    if (arena->capacity > arena->size + size) {
        ptr = &arena->memory[arena->size];
        arena->size += size;
        return ptr;
    }
    printf("There is no more area in arena. Increase the capacity!\n");
    return NULL;
}

void* arena_realloc(arena_t* arena, u64_t size) {
    // TODO: implement this
    (void)(arena);
    (void)(size);
    return NULL;
}

void arena_reset(arena_t* arena) {
    arena->size = 0;
}

void arena_destroy(arena_t* arena) {
    arena->size = 0;
    arena->capacity = 0;
    free(arena->memory);
}



#endif//ARENA_IMPLEMENTATION

#endif//ARENA_H

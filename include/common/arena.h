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
    struct arena_struct* prev;
    struct arena_struct* next;
} arena_t;

arena_t* arena_init(u64_t size);
void* arena_alloc(arena_t* arena, u64_t size);
void* arena_realloc(arena_t* arena, void* ptr, u64_t size);
void arena_grow(arena_t* arena);
void arena_reset(arena_t* arena);
void arena_destroy(arena_t* arena);


#ifdef ARENA_IMPLEMENTATION
#undef ARENA_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

arena_t* arena_init(u64_t size) {
    arena_t arena = {
        .prev = NULL,
        .next = NULL,
        .capacity = size,
        .size = 0,
        .memory = (u8_t*)malloc(size)
    };
    arena_t* ptr = malloc(sizeof(arena_t));
    *ptr = arena;
    return ptr;
}

void* arena_alloc(arena_t* arena, u64_t size) {
    if (size == 0)
        return NULL;
    if (arena->size > arena->capacity - size)
        arena_grow(arena);
    void* ptr = &arena->memory[arena->size];
    arena->size += size;
    return ptr;
}

void* arena_realloc(arena_t* arena, void* ptr, u64_t new_size) {
    if (ptr == NULL) {
        return arena_alloc(arena, new_size);
    }
    return NULL;
}

void arena_grow(arena_t* arena) {
    arena->next = arena_init(arena->capacity);
    arena->next->prev = arena;
    arena = arena->next;
}

void arena_reset(arena_t* arena) {
    arena->size = 0;
}

void arena_destroy(arena_t* arena) {
    if (arena == NULL)
        return;
    if (arena->next)
        free(arena->next);
        
    free(arena->memory);
    arena_destroy(arena->prev);
}



#endif//ARENA_IMPLEMENTATION

#endif//ARENA_H

#ifndef ARENA_H
#define ARENA_H

#ifdef ARENA_NO_STDINT

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long   uint64_t;
typedef signed   char   int8_t;
typedef signed   short  int16_t;
typedef signed   int    int32_t;
typedef signed   long   int64_t;

#else
#include <stdint.h>
#include <stddef.h>
#endif//ARENA_NO_STDINT

typedef struct arena_struct {
    size_t size;
    size_t capacity;
    uint8_t* memory;
    struct arena_struct* prev;
    struct arena_struct* next;
} arena_t;

arena_t* arena_init(size_t size);
void* arena_alloc(arena_t* arena, size_t size);
void* arena_realloc(arena_t* arena, void* ptr, size_t size);
arena_t* arena_grow(arena_t* arena);
void arena_reset(arena_t* arena);
void arena_destroy(arena_t* arena);


#ifdef ARENA_IMPLEMENTATION
#undef ARENA_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

arena_t* arena_init(size_t size) {
    arena_t arena = {
        .prev = NULL,
        .next = NULL,
        .capacity = size,
        .size = 0,
        .memory = (uint8_t*)malloc(size)
    };
    arena_t* ptr = malloc(sizeof(arena_t));
    *ptr = arena;
    return ptr;
}

void* arena_alloc(arena_t* arena, size_t size) {
    if (size == 0)
        return NULL;
    if (arena->size + size > arena->capacity)
        arena = arena_grow(arena);
    void* ptr = &arena->memory[arena->size];
    arena->size += size;
    return ptr;
}

void* arena_realloc(arena_t* arena, void* ptr, size_t new_size) {
    if (ptr == NULL) {
        return arena_alloc(arena, new_size);
    }
    // TODO: not implemented yet!
    return NULL;
}

arena_t* arena_grow(arena_t* arena) {
    arena->next = arena_init(arena->capacity);
    arena->next->prev = arena;
    arena = arena->next;
    return arena;
}

void arena_reset(arena_t* arena) {
    arena->size = 0;
}

void arena_destroy(arena_t* arena) {
    arena_t** a = &arena;
    while ((*a)->prev != NULL) {
        free((*a)->memory);
        (*a) = (*a)->prev;
    }
    free((*a)->memory);
    free(*a);
}



#endif//ARENA_IMPLEMENTATION

#endif//ARENA_H

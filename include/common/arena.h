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
void* arena_realloc(arena_t* arena, void* ptr, u64_t size);
void arena_resize(arena_t* arena, u64_t size);
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
    if (arena->capacity <= arena->size + size) {
        arena_resize(arena, arena->size + size);
    }
    ptr = &arena->memory[arena->size];
    arena->size += size;
    return ptr;
}

void* arena_realloc(arena_t* arena, void* ptr, u64_t new_size) {
    if (ptr == NULL) {
        return arena_alloc(arena, new_size);
    }

    u8_t* old_ptr = (u8_t*)ptr;
    size_t old_offset = old_ptr - arena->memory;
    size_t current_size = arena->size - old_offset;

    // If expanding within available space
    if (old_offset + new_size <= arena->size) {
        // Check if it overlaps with the next allocation
        if (old_offset + new_size > old_offset + current_size) {
            memmove(&arena->memory[arena->size], old_ptr, current_size);
            arena->size += new_size - current_size;
        }
        return old_ptr;
    }

    // If we need to expand beyond current size, check capacity
    if (old_offset + new_size > arena->capacity) {
        arena_resize(arena, arena->capacity * 2);
    }

    // Move the block to a new location
    void* new_ptr = arena_alloc(arena, new_size);
    if (new_ptr) {
        memcpy(new_ptr, old_ptr, current_size);
        arena->size = old_offset + new_size;
    }

    return new_ptr;
}

void arena_resize(arena_t* arena, u64_t size) {
    arena->memory = (u8_t*)realloc(arena->memory, size);
    arena->capacity = size;
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

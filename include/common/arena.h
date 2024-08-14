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
    if (arena->capacity > arena->size + size) {
        ptr = &arena->memory[arena->size];
        arena->size += size;
        return ptr;
    }
    printf("There is no more area in arena. Increase the capacity!\n");
    return NULL;
}

void* arena_realloc(arena_t* arena, void* ptr, u64_t size) {
    if (ptr == NULL) {
        return arena_alloc(arena, size); // If ptr is NULL, just allocate new memory.
    }
    
    u8_t* old_ptr = (u8_t*)ptr;
    u64_t old_offset = old_ptr - arena->memory;
    
    if (old_offset + size <= arena->capacity) {
        // If the requested size is within the current arena capacity, simply adjust size.
        arena->size = old_offset + size;
        return old_ptr;
    }

    // Otherwise, allocate new memory within the arena.
    void* new_ptr = arena_alloc(arena, size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, arena->size - old_offset);
    } else {
        printf("Reallocation failed: Not enough space in the arena.\n");
    }
    return new_ptr;
}

void arena_resize(arena_t* arena, u64_t size) {
    if (size > arena->capacity) {
        u8_t* new_memory = (u8_t*)malloc(size);
        if (new_memory == NULL) {
            printf("Resize failed!\n");
            return;
        }
        memcpy(new_memory, arena->memory, arena->size);
        free(arena->memory);
        arena->memory = new_memory;
        arena->capacity = size;
    }
    if (size < arena->size) {
        arena->size = size;
    }
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

#ifndef TGC_H_
#define TGC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

_Static_assert(sizeof(void*) == sizeof(uintptr_t), "Incompetible pointer size on the current architecture!");

typedef struct gc_block gc_block;

struct gc_block {
    /*
        The placement and allignment of this struct is fucking very important
        We do so many dangerous memory arithmetic with them,
        and all of them is managed by our custom assembly language called TASM which is now more dangerous than good old C
        despite it is written in C
    */
    void* value;
    uint64_t pointer_count;
    uint64_t size; // size of allocated memory in bytes
    gc_block* pointers;
    int64_t marked;
};

uintptr_t tgc_create_block(size_t size, size_t pointer_count);
void tgc_mark(void* ptr);
void tgc_sweep();

#ifdef TGC_IMPLEMENTATION

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

gc_block* __heap = NULL;
size_t __heap_block_count = 0;

uintptr_t tgc_create_block(size_t size, size_t pointer_count) {
    gc_block block = (gc_block) {
        .size = size,
        .value = malloc(size),
        .pointers = NULL,
        .pointer_count = pointer_count,
        .marked = false,
    };
    // put the block to the heap
    arrput(__heap, block);

    // add pointers after the allocated the block
    if (pointer_count != 0)
        arraddnptr(__heap, pointer_count);
    
    uintptr_t block_addr = (uintptr_t)&__heap[__heap_block_count];
    __heap_block_count += pointer_count + 1;

    if ((uintptr_t)block.value % 8 != 0) {
        fprintf(stderr, "tgc_create_block: Misaligned value=%p\n", block.value);
    }

    printf("heap_size: %llu\n", __heap_block_count);
    // TODO: set the array capacity of heap bigger than 10 (10 is default, use arrsetcap)
    printf("cap: %llu\n", arrcap(__heap));
    printf("last_block_addr: %p\n", block_addr);
    printf("sizeof(uintptr_t): %d\nsizeof(uint64_t): %d\n", sizeof(uintptr_t), sizeof(uint64_t));
    return block_addr;
}

void tgc_mark(void* root) {
    if (root == NULL) return;
    gc_block* block = (gc_block*)root;
    if (block->marked) return; // Already marked

    block->marked = 1;

    // Recursively mark pointers inside the block
    for (size_t i = 0; i < arrlenu(block->pointers); i++) {
        void* child = &block->pointers[i];
        tgc_mark(child);
    }
}

void tgc_sweep() {
    for (size_t i = 0; i < arrlenu(__heap); i++) {
        if (__heap[i].marked) {
            __heap[i].marked = 0;
            printf("marked!\n");
        } else {
            arrdel(__heap, i);
            __heap_block_count--;
            if (__heap[i].value)
                free(__heap[i].value);
            printf("unmarked deleted\n");
        }
    }
}

void tgc_destroy() {
    // tgc_sweep();
    arrfree(__heap);
}


#endif//TGC_IMPLEMENTATION

#endif//TGC_H_
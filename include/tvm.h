#ifndef TVM_H_
#define TVM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TVM_STACK_CAPACITY 1024
#define TVM_PROGRAM_CAPACITY 1024

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef enum {
    EXCEPT_OK,
    EXCEPT_STACK_UNDERFLOW,
    EXCEPT_STACK_OVERFLOW,
    EXCEPT_INVALID_INSTRUCTION,
    EXCEPT_DIVISION_BY_ZERO,
} exception_t;

typedef uint32_t word_t;

typedef enum {
    OP_NOP,
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    OP_MOD,
    OP_DUP,
    OP_ADDF,
    OP_SUBF,
    OP_MULTF,
    OP_DIVF,

    /* branching */
    OP_JMP,  // unconditional jump
    OP_JNZ,  // conditional jump
    OP_HALT, // termination
} optype_t;

typedef union {
    uint32_t ui32; // usually for addresses
    int32_t  i32;
    float    f32;
} object_t;

typedef struct {
    optype_t type;
    object_t operand;
} opcode_t;

typedef struct {
    object_t stack[TVM_STACK_CAPACITY];
    word_t sp; // stack pointer

    opcode_t program[TVM_PROGRAM_CAPACITY];
    size_t program_size;
    word_t ip; // instruction pointer

    bool halted;
} tvm_t;



void tvm_load_program_from_memory(tvm_t* vm, const opcode_t* program, size_t program_size);
void tvm_save_program_to_memory(tvm_t* vm, opcode_t* program);
void tvm_load_program_from_file(tvm_t* vm, const char* file_path);
void tvm_save_program_to_file(tvm_t* vm, const char* file_path);
const char* exception_to_cstr(exception_t except);
tvm_t tvm_init();
exception_t tvm_exec_opcode(tvm_t* vm);
void tvm_run(tvm_t* vm);
void tvm_stack_dump(tvm_t* vm);



#ifdef TVM_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tvm_load_program_from_memory(tvm_t* vm, const opcode_t* program, size_t program_size) {
    vm->program_size = program_size;
    memcpy(vm->program, program, vm->program_size * sizeof(vm->program[0]));
}

void tvm_save_program_to_memory(tvm_t* vm, opcode_t* program) {

}

void tvm_load_program_from_file(tvm_t* vm, const char* file_path) {

}

void tvm_save_program_to_file(tvm_t* vm, const char* file_path) {

}


const char* exception_to_cstr(exception_t except) {
    switch (except)
    {
    case EXCEPT_OK:
        return "EXCEPT_OK";
    case EXCEPT_STACK_UNDERFLOW:
        return "EXCEPT_STACK_UNDERFLOW";
    case EXCEPT_STACK_OVERFLOW:
        return "EXCEPT_STACK_OVERFLOW";
    case EXCEPT_INVALID_INSTRUCTION:
        return "EXCEPT_INVALID_INSTRUCTION";
    case EXCEPT_DIVISION_BY_ZERO:
        return "EXCEPT_DIVISION_BY_ZERO";
    
    default:
        fprintf(stderr, "Unhandled exception string on function: exception_to_cstr\n");
        break;
    }
}

tvm_t tvm_init() {
    return (tvm_t) {
        .stack = {0},
        .sp = 0,
        .program = {0},
        .program_size = 0,
        .ip = 0,
        .halted = 0,
    };
}

exception_t tvm_exec_opcode(tvm_t* vm) {
    opcode_t inst = vm->program[vm->ip];
    // printf("inst: %d\n", inst.type);
    tvm_stack_dump(vm);
    switch (inst.type) {
    case OP_NOP:
        /* no operation */
        vm->ip++;
        break;
    case OP_PUSH:
        vm->stack[vm->sp++] = inst.operand;
        vm->ip++;
        break;
    case OP_ADD:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].i32 += vm->stack[vm->sp - 1].i32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_SUB:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].i32 -= vm->stack[vm->sp - 1].i32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_MULT:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].i32 *= vm->stack[vm->sp - 1].i32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_DIV:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 == 0)
            return EXCEPT_DIVISION_BY_ZERO;
        vm->stack[vm->sp - 2].i32 /= vm->stack[vm->sp - 1].i32;
        vm->sp--;
        vm->ip++;
        break;
        case OP_MOD:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 == 0)
            return EXCEPT_DIVISION_BY_ZERO;
        vm->stack[vm->sp - 2].i32 %= vm->stack[vm->sp - 1].i32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_DUP:
        /* not implemented yet */
        vm->ip++;
        break;
    case OP_ADDF:
        /* not implemented yet */
        break;
        vm->ip++;
    case OP_SUBF:
        /* not implemented yet */
        break;
        vm->ip++;
    case OP_MULTF:
        /* not implemented yet */
        break;
        vm->ip++;
    case OP_DIVF:
        /* not implemented yet */
        break;
        vm->ip++;
    case OP_JMP:
        /* not implemented yet */
        vm->ip++;
        break;
    case OP_JNZ:
        /* not implemented yet */
        vm->ip++;
        break;
    case OP_HALT:
        vm->halted = true;
        vm->ip++;
        break;
    
    default:
        return EXCEPT_INVALID_INSTRUCTION;
        break;
    }

    return EXCEPT_OK;
}

void tvm_run(tvm_t* vm) {
    while (!vm->halted && vm->ip <= vm->program_size) {
        exception_t except = tvm_exec_opcode(vm);
        if (except != EXCEPT_OK) {
            fprintf(stderr, "ERROR: Exception occured %s\n", exception_to_cstr(except));
            exit(1);
        }
    }
    fprintf(stdout, "Program halted succesfully...\n");
}

void tvm_stack_dump(tvm_t *vm) {
    fprintf(stdout, "stack:\n");
    for (size_t i = 0; i < vm->sp; i++) {
        fprintf(stdout, "0x%08x: %d\n", i, vm->stack[i]);
    }
}

#endif//TVM_IMPLEMENTATION
#endif//TVM_H_
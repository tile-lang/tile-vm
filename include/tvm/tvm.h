#ifndef TVM_H_
#define TVM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ARENA_IMPLEMENTATION
#include <common/arena.h>

#define TVM_STACK_CAPACITY 1024
#define TVM_PROGRAM_CAPACITY 1024
#define TVM_METADATA_CAPACITY 512
#define RETURN_STACK_CAPACITY 1024

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define UNUSED_VAR(x) ((void)(x))

typedef enum {
    EXCEPT_OK,
    EXCEPT_STACK_UNDERFLOW,
    EXCEPT_STACK_OVERFLOW,
    EXCEPT_INVALID_INSTRUCTION,
    EXCEPT_INVALID_INSTRUCTION_ACCESS,
    EXCEPT_INVALID_STACK_ACCESS,
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
    OP_CLN,
    OP_SWAP,
    OP_ADDF,
    OP_SUBF,
    OP_MULTF,
    OP_DIVF,
    OP_INC,
    OP_INCF,
    OP_DEC,
    OP_DECF,
    /* branching */
    OP_JMP,  // unconditional jump
    OP_JZ,   // conditional jump
    OP_JNZ,  // conditional jump
    OP_CALL,
    OP_RET,
    /* casting */
    OP_CI2F,
    OP_CI2U,
    OP_CF2I,
    OP_CF2U,
    OP_CU2I,
    OP_CU2F,
    /* comparison */
    OP_GT,
    OP_GTF,
    OP_LT,
    OP_LTF,
    OP_EQ,
    OP_EQF,
    OP_GE,
    OP_GEF,
    OP_LE,
    OP_LEF,
    OP_HALT // termination
} optype_t;

typedef struct {
    enum {
        OBJECT_TYPE_ADDRESS,
        OBJECT_TYPE_NUMBER,
    } type;
    union {
        uint32_t ui32; // usually for addresses
        int32_t  i32;
        float    f32;
    };
} object_t;

typedef struct {
    optype_t type;
    object_t operand;
} opcode_t;

typedef enum {
    CTYPE_UINT8,
    CTYPE_UINT16,
    CTYPE_UINT32,
    CTYPE_UINT64,
    CTYPE_INT8,
    CTYPE_INT16,
    CTYPE_INT32,
    CTYPE_INT64,
    CTYPE_FLOAT32,
    CTYPE_FLOAT64,
    CTYPE_PTR,
    CTYPE_VOID,
} tvm_ctype;

typedef struct {
    uint8_t  rtype;           // return type
    uint8_t* atypes;          // arg types
    uint16_t acount;          // arg count
    const char* symbol_name;  // function name
} tvm_program_cfun_t;
//TODO: support structs and function pointers, it will be much more complicated. it is now only primitives

// typedef struct tvm_program_cstruct {
//     tvm_ctype* etypes;                  // element types
//     struct tvm_program_cstruct* ctypes; // composite types
//     const char* symbol_name;            // struct name
// } tvm_program_cstruct_t;

typedef struct {
    int date, hour; // created at.
    tvm_program_cfun_t cfuns[TVM_METADATA_CAPACITY];
    uint32_t cfun_count;
} tvm_program_metadata_t;

typedef struct {
    tvm_program_metadata_t metadata;
    opcode_t code[TVM_PROGRAM_CAPACITY];
    size_t size;
    arena_t* program_arena;
} tvm_program_t;

typedef struct {
    object_t stack[TVM_STACK_CAPACITY];
    word_t sp; // stack pointer

    word_t return_stack[RETURN_STACK_CAPACITY];
    word_t rsp; // return stack pointer

    tvm_program_t program;
    word_t ip; // instruction pointer

    bool halted;
} tvm_t;



void tvm_load_program_from_memory(tvm_t* vm, const opcode_t* code, size_t program_size);
void tvm_save_program_to_memory(tvm_t* vm, opcode_t* code);
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

#include <tvm/tci.h>

void tvm_load_program_from_memory(tvm_t* vm, const opcode_t* code, size_t program_size) {
    vm->program.size = program_size;
    memcpy(vm->program.code, code, vm->program.size * sizeof(vm->program.code[0]));
}

void tvm_save_program_to_memory(tvm_t* vm, opcode_t* program) {
    UNUSED_VAR(vm);
    UNUSED_VAR(program);
}

void tvm_load_program_from_file(tvm_t* vm, const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    
    fseek(file,0L,SEEK_END);
    long int byte_size = ftell(file);
    fseek(file,0L,SEEK_SET);
    size_t opcode_size = sizeof(vm->program.code[0]);

    uint32_t cfun_count;
    fread(&cfun_count, sizeof(uint32_t), 1, file);
    byte_size -= sizeof(uint32_t);
    vm->program.metadata.cfun_count = cfun_count;
    {
        for (size_t i = 0; i < cfun_count; i++) {    
            uint8_t symbol_name_len;
            fread(&symbol_name_len, sizeof(uint8_t), 1, file);
            byte_size -= sizeof(uint8_t);

            char* symbol_name = arena_alloc(vm->program.program_arena, sizeof(char) * symbol_name_len + 1);
            fread(symbol_name, sizeof(char), symbol_name_len, file);
            byte_size -= sizeof(char) * symbol_name_len;

            uint16_t acount;
            fread(&acount, sizeof(uint16_t), 1, file);
            byte_size -= sizeof(uint16_t);

            uint8_t rtype;
            fread(&rtype, sizeof(uint8_t), 1, file);
            byte_size -= sizeof(uint8_t);

            uint8_t* atypes = arena_alloc(vm->program.program_arena, sizeof(uint8_t) * acount);
            fread(atypes, sizeof(uint8_t), acount, file);
            byte_size -= sizeof(uint8_t) * acount;

            vm->program.metadata.cfuns[i].symbol_name = symbol_name;
            vm->program.metadata.cfuns[i].acount = acount;
            vm->program.metadata.cfuns[i].rtype = rtype;
            vm->program.metadata.cfuns[i].atypes = atypes;
        }
    }
    vm->program.size = byte_size/opcode_size;
    fread(vm->program.code, opcode_size, vm->program.size, file);
    
    fclose(file);
}


void tvm_save_program_to_file(tvm_t* vm, const char* file_path) {
    UNUSED_VAR(vm);
    UNUSED_VAR(file_path);
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
    case EXCEPT_INVALID_INSTRUCTION_ACCESS:
        return "EXCEPT_INVALID_INSTRUCTION_ACCESS";
    case EXCEPT_INVALID_STACK_ACCESS:
        return "EXCEPT_INVALID_STACK_ACCESS";
    case EXCEPT_DIVISION_BY_ZERO:
        return "EXCEPT_DIVISION_BY_ZERO";
        
    default:
        fprintf(stderr, "Unhandled exception string on function: exception_to_cstr\n");
        break;
    }
    return NULL;
}

tvm_t tvm_init() {
    return (tvm_t) {
        .stack = {0},
        .sp = 0,
        .return_stack = {0},
        .rsp = 0,
        .program = {
            .metadata = {
                .date = 0,
                .hour = 0,
                .cfuns = {0},
            },
            .code = {0},
            .size = 0,
            .program_arena = arena_init(1024),
        },
        .ip = 0,
        .halted = 0,
    };
}

exception_t tvm_exec_opcode(tvm_t* vm) {
    opcode_t inst = vm->program.code[vm->ip];
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
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp].i32 = vm->stack[vm->sp - 1].i32;
        vm->sp++;
        vm->ip++;
        break;
    case OP_CLN:
        if (inst.operand.ui32 >= (uint32_t)vm->sp)
            return EXCEPT_INVALID_STACK_ACCESS;
        else if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp] = vm->stack[vm->sp - inst.operand.ui32 - 1];
        vm->sp++;
        vm->ip++;
        break;
    case OP_SWAP:
        if (inst.operand.ui32 >= (uint32_t)vm->sp)
            return EXCEPT_INVALID_STACK_ACCESS;
        else if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        object_t temp = vm->stack[vm->sp - 1];
        vm->stack[vm->sp - 1] = vm->stack[vm->sp - inst.operand.ui32 - 1];
        vm->stack[vm->sp - inst.operand.ui32 - 1] = temp;
        vm->ip++;
        break;
    case OP_ADDF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].f32 += vm->stack[vm->sp - 1].f32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_SUBF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].f32 -= vm->stack[vm->sp - 1].f32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_MULTF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].f32 *= vm->stack[vm->sp - 1].f32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_DIVF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        vm->stack[vm->sp - 2].f32 /= vm->stack[vm->sp - 1].f32;
        vm->sp--;
        vm->ip++;
        break;
    case OP_INC:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].i32 += 1;
        vm->ip++;
        break;
    case OP_INCF:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].f32 += 1;
        vm->ip++;
        break;
    case OP_DEC:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].i32 -= 1;
        vm->ip++;
        break;
    case OP_DECF:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].f32 -= 1;
        vm->ip++;
        break;
    case OP_JMP:
        if (inst.operand.ui32 >= vm->program.size)
            return EXCEPT_INVALID_INSTRUCTION_ACCESS;
        vm->ip = inst.operand.ui32;
        // vm->ip++; you can delete this comment
        break;
    case OP_JZ:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        else if (inst.operand.ui32 >= vm->program.size)
            return EXCEPT_INVALID_INSTRUCTION_ACCESS;
        if (vm->stack[vm->sp - 1].ui32 == 0)
            vm->ip = inst.operand.ui32;
        else 
            vm->ip++;
        break;
    case OP_JNZ:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        else if (inst.operand.ui32 >= vm->program.size)
            return EXCEPT_INVALID_INSTRUCTION_ACCESS;
        if (vm->stack[vm->sp - 1].ui32 != 0)
            vm->ip = inst.operand.ui32;
        else 
            vm->ip++;
        break;
    case OP_CALL:
        if (vm->rsp >= RETURN_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        else if (inst.operand.ui32 >= vm->program.size)
            return EXCEPT_INVALID_INSTRUCTION_ACCESS;
        vm->return_stack[vm->rsp++] = vm->ip + 1;
        vm->ip = inst.operand.ui32;
        break; 
    case OP_RET:
        if (vm->rsp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->ip = vm->return_stack[--vm->rsp];
        break;
    case OP_CI2F:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].f32 = vm->stack[vm->sp - 1].i32;  
        vm->ip++;
        break;
    case OP_CI2U:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].ui32 = vm->stack[vm->sp - 1].i32;  
        vm->ip++;
        break;        
    case OP_CF2I:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].i32 = vm->stack[vm->sp - 1].f32;  
        vm->ip++;
        break;
    case OP_CF2U:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].ui32 = vm->stack[vm->sp - 1].f32;  
        vm->ip++;
        break;
    case OP_CU2I:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].i32 = vm->stack[vm->sp - 1].ui32;  
        vm->ip++;
        break;
    case OP_CU2F:
        if (vm->sp < 1)
            return EXCEPT_STACK_UNDERFLOW;
        vm->stack[vm->sp - 1].f32 = vm->stack[vm->sp - 1].ui32;  
        vm->ip++;
        break;
    case OP_GT:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 > vm->stack[vm->sp - 2].i32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++; 
        break;
    case OP_GTF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].f32 > vm->stack[vm->sp - 2].f32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_LT:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 < vm->stack[vm->sp - 2].i32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_LTF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].f32 < vm->stack[vm->sp - 2].f32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_EQ:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 == vm->stack[vm->sp - 2].i32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_EQF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].f32 == vm->stack[vm->sp - 2].f32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_GE:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 >= vm->stack[vm->sp - 2].i32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_GEF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].f32 >= vm->stack[vm->sp - 2].f32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_LE:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].i32 <= vm->stack[vm->sp - 2].i32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
        vm->ip++;
        break;
    case OP_LEF:
        if (vm->sp < 2)
            return EXCEPT_STACK_UNDERFLOW;
        else if (vm->sp >= TVM_STACK_CAPACITY)
            return EXCEPT_STACK_OVERFLOW;
        if (vm->stack[vm->sp - 1].f32 <= vm->stack[vm->sp - 2].f32)
            vm->stack[vm->sp].i32 = 1;
        else
            vm->stack[vm->sp].i32 = 0;
        vm->sp++;
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
    while (!vm->halted && vm->ip <= vm->program.size) {
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
        fprintf(stdout, "0x%08x: %d (as int), %f (as float)\n", i, vm->stack[i].i32, vm->stack[i].f32); // Ask 0x%08zx you can delete this comment
    }
}

#endif//TVM_IMPLEMENTATION
#endif//TVM_H_
#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>

opcode_t program[] = {
    {.type = OP_PUSH, .operand.i32 = 1}, 
    {.type = OP_PUSH, .operand.i32 = 2},  
    {.type = OP_PUSH, .operand.i32 = 3},
    {.type = OP_JNZ, .operand.i32 = 11},                 
    {.type = OP_PUSH, .operand.i32 = 4},
    {.type = OP_PUSH, .operand.i32 = 5},
    {.type = OP_PUSH, .operand.i32 = 6},
    {.type = OP_PUSH, .operand.i32 = 7},
    {.type = OP_PUSH, .operand.i32 = 8},
    {.type = OP_PUSH, .operand.i32 = 9},
    {.type = OP_HALT}                   
};

int main(int argc, char **argv) {
    UNUSED_VAR(argc);
    UNUSED_VAR(argv);
    
    tvm_t vm = tvm_init();
    tvm_load_program_from_memory(&vm, program, 9);
    tvm_run(&vm);

    return 0;
}
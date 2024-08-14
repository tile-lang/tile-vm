#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>

opcode_t program[] = {
    // Main Program
    {.type = OP_PUSH, .operand.i32 = 4},
    {.type = OP_PUSH, .operand.ui32 = 2},
    {.type = OP_PUSH, .operand.f32 = 3.4},
    {.type = OP_PUSH, .operand.f32 = 7.4},
    {.type = OP_PUSH, .operand.i32 = 6},
    {.type = OP_PUSH, .operand.f32 = 8.4},
    {.type = OP_CLN, .operand.i32 = 12},
    {.type = OP_HALT}               
};



int main(int argc, char **argv) {
    UNUSED_VAR(argc);
    UNUSED_VAR(argv);
    
    tvm_t vm = tvm_init();
    //tvm_load_program_from_memory(&vm, program, 15);
    tvm_load_program_from_file(&vm, "out.bin");
    tvm_run(&vm);

    return 0;
}
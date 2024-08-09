#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>

opcode_t program[] = {
    {.type = OP_PUSH, .operand.i32 = 16},
    {.type = OP_PUSH, .operand.f32 = 2.4},
    {.type = OP_CI2F},
    {.type = OP_PUSH, .operand.i32 = 34},
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
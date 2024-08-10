#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>

opcode_t program[] = {
    {.type = OP_PUSH, .operand.i32 = 17},
    {.type = OP_PUSH, .operand.f32 = 2.4},
    {.type = OP_DECF},
    {.type = OP_PUSH, .operand.f32 = -7.89},
    {.type = OP_INCF},
    {.type = OP_PUSH, .operand.i32 = -11},
    {.type = OP_INC},
    {.type = OP_PUSH, .operand.i32 = -85},
    {.type = OP_DEC},
    {.type = OP_PUSH, .operand.f32 = 3.4},
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
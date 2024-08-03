#define TVM_IMPLEMENTATION
#include <tvm.h>

opcode_t program[] = {
    {.type = OP_PUSH, .operand = 3},
    {.type = OP_PUSH, .operand = 5},
    {.type = OP_ADD},
    {.type = OP_HALT},
};

int main(int argc, char **argv) {
    UNUSED_VAR(argc);
    UNUSED_VAR(argv);
    
    tvm_t vm = tvm_init();
    tvm_load_program_from_memory(&vm, program, 4);
    tvm_run(&vm);

    return 0;
}
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <tasmc/tasmc.h>
#include <tasmc/tasmc_target.h>

static FILE* stream;

static void op_file() {
    fprintf(
        stream,
        "BITS 64\n"
        TASMC_TARGET_x86_64_WIN32
        "section .text\n"
        "global _start\n"
        "\n" \
        "_start:\n"
        _T"xor rsi, rsi\n"
        _T"mov rbp, stack\n"
    );
}

static void op_stack() {
    fprintf(
        stream,
        TASMC_TARGET_STACK
    );
}

static void op_nop() {
    fprintf(
        stream,
        _T"; nop\n"
        _T"nop\n"
    );
}

static void op_push(const char* operand) {
    printf("%s\n", operand);
    fprintf(
        stream,
        _T"; push %s\n"
        _T"mov rax, %s\n"
        _T"mov qword [rbp + rsi], rax\n"
        _T"add rsi, 4\n"
        _T"xor rax, rax\n",
        operand,
        operand
    );
}

static void op_pop() {
    fprintf(
        stream,
        _T"; pop\n"
        _T"sub rsi, 4\n"
        _T"mov qword [rbp + rsi], rax\n"
    );
}

static void op_hlt() {
    fprintf(
        stream,
        _T"; hlt\n"
        _T"push qword 0\n"
        _T"xor rcx, rcx\n"
        _T"call ExitProcess\n"
    );
}

static void op_add() {
    fprintf(
        stream,
        _T"; add\n"
        _T"sub rsi, 4\n"
        _T"mov rbx, qword [rbp + rsi]\n"
        _T"sub rsi, 4\n"
        _T"mov rax, qword [rbp + rsi]\n"
        _T"add rax, rbx\n"
        _T"mov qword [rbp + rsi], rax\n"
        _T"add rsi, 4\n"
        _T"xor rax, rax\n"
    );
}

void tasmc_init(const char* output) {
    stream = fopen(output, "w+");
}

void tasmc_destroy() {
    fclose(stream);
}

static void tasmc_compile_nasm_code(tasm_ast_t* node) {
    if (!node) return;

    switch (node->tag) {
        case AST_NONE: assert(false && "unreachable!"); break;
        case AST_FILE:
            op_file();
            for (size_t i = 0; i < node->file.line_size; i++)
                tasmc_compile_nasm_code(node->file.lines[i]);
            break;
        case AST_OP_NOP: op_nop(); break;
        case AST_OP_PUSH: op_push(node->inst.operand->number.text_value); break;
        case AST_OP_POP: op_pop(); break;
        case AST_OP_ADD: op_add(); break;
        case AST_OP_SUB: break;
        case AST_OP_MULT: break;
        case AST_OP_DIV: break;
        case AST_OP_MOD: break;
        case AST_OP_DUP: break;
        case AST_OP_CLN: break;
        case AST_OP_SWAP: break;
        case AST_OP_ADDF: break;
        case AST_OP_SUBF: break;
        case AST_OP_MULTF: break;
        case AST_OP_DIVF: break;
        case AST_OP_INC: break;
        case AST_OP_INCF: break;
        case AST_OP_DEC: break;
        case AST_OP_DECF: break;
        case AST_OP_JMP: break;
        case AST_OP_JZ: break;
        case AST_OP_JNZ: break;
        case AST_OP_CALL: break;
        case AST_OP_RET: break;
        case AST_OP_CI2F: break;
        case AST_OP_CI2U: break;
        case AST_OP_CF2I: break;
        case AST_OP_CF2U: break;
        case AST_OP_CU2I: break;
        case AST_OP_CU2F: break;
        case AST_OP_GT: break;
        case AST_OP_GTF: break;
        case AST_OP_LT: break;
        case AST_OP_LTF: break;
        case AST_OP_EQ: break;
        case AST_OP_EQF: break;
        case AST_OP_GE: break;
        case AST_OP_GEF: break;
        case AST_OP_LE: break;
        case AST_OP_LEF: break;
        case AST_OP_LOADC: break;
        case AST_OP_ALOADC: break;
        case AST_OP_LOAD: break;
        case AST_OP_STORE: break;
        case AST_OP_PUTS: break;
        case AST_OP_NATIVE: break;
        case AST_OP_HALT: op_hlt(); break;
        case AST_NUMBER:
            printf("NUMBER: %s\n", node->number.text_value);
            break;
        case AST_CHAR:
            printf("CHAR: %s\n", node->character.value);
            break;
        case AST_STRING:
            printf("STRING: \"%s\" length: %d\n", node->string.value, node->string.length);
            break;
        case AST_LABEL_DECL:
            printf("LABEL DECL: %s\n", node->label_decl.name);
            break;
        case AST_LABEL_CALL:
            printf("LABEL CALL: %s\n", node->label_call.name);
            break;
        case AST_PROC:
            printf("PROC %s\n", node->proc.name);
            for (size_t i = 0; i < node->proc.line_size; i++) {
                tasmc_compile_nasm_code(node->proc.lines[i]);
            }
            break;

        case AST_DATA:
            printf("DATA\n");
            tasmc_compile_nasm_code(node->data.value);
            break;
        case AST_CSTRUCT:
            break;
        case AST_CFUNCTION:
            printf("CFUNCTION %s: ", node->cfunction.name);
            printf("%d -> (", node->cfunction.ret_type);
            for (size_t i = 0; i < node->cfunction.arg_count; i++) {
                printf("%d, ", node->cfunction.arg_types[i]);
            }
            printf(")\n");
            break;
        
        default:
            assert(false && "unreachable!"); break;
    }
}

static void tasmc_compile_nasm_end() {
    op_stack();
}

void tasmc_compile_nasm(tasm_ast_t* node) {
    tasmc_compile_nasm_code(node);
    tasmc_compile_nasm_end();
}

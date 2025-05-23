#ifndef TASM_AST_H_
#define TASM_AST_H_

#include <tasm/tasm_lexer.h>
#include <stb_ds.h>

typedef enum {
    AST_CTYPE_UINT8,
    AST_CTYPE_UINT16,
    AST_CTYPE_UINT32,
    AST_CTYPE_UINT64,
    AST_CTYPE_INT8,
    AST_CTYPE_INT16,
    AST_CTYPE_INT32,
    AST_CTYPE_INT64,
    AST_CTYPE_PTR,
    AST_CTYPE_VOID,
} tasm_ast_ctype;

typedef struct tasm_ast{
    enum {
        AST_NONE,
        AST_FILE,

        AST_CFUNCTION,
        AST_CSTRUCT,
        AST_DATA,

        AST_OP_NOP,
        AST_OP_PUSH,
        AST_OP_POP,
        AST_OP_ADD,
        AST_OP_SUB,
        AST_OP_MULT,
        AST_OP_DIV,
        AST_OP_MOD,
        AST_OP_DUP,
        AST_OP_CLN,
        AST_OP_SWAP,
        AST_OP_ADDF,
        AST_OP_SUBF,
        AST_OP_MULTF,
        AST_OP_DIVF,
        AST_OP_INC,
        AST_OP_INCF,
        AST_OP_DEC,
        AST_OP_DECF,
        AST_OP_JMP,
        AST_OP_JZ,
        AST_OP_JNZ,
        AST_OP_CALL,
        AST_OP_RET,
        AST_OP_CI2F,
        AST_OP_CI2U,
        AST_OP_CF2I,
        AST_OP_CF2U,
        AST_OP_CU2I,
        AST_OP_CU2F,
        AST_OP_GT,
        AST_OP_GTF,
        AST_OP_LT,
        AST_OP_LTF,
        AST_OP_EQ,
        AST_OP_EQF,
        AST_OP_GE,
        AST_OP_GEF,
        AST_OP_LE,
        AST_OP_LEF,
        AST_OP_AND,
        AST_OP_OR,
        AST_OP_NOT,
        AST_OP_BAND,
        AST_OP_BOR,
        AST_OP_BNOT,
        AST_OP_LSHFT,
        AST_OP_RSHFT,
        AST_OP_LOADC,
        AST_OP_ALOADC,
        AST_OP_LOAD,
        AST_OP_STORE,
        AST_OP_GLOAD,
        AST_OP_GSTORE,
        AST_OP_HALLOC,
        AST_OP_DEREF,
        AST_OP_DEREFB,
        AST_OP_HSET,
        AST_OP_HSETOF,
        AST_OP_PUTS,
        AST_OP_PUTC,
        AST_OP_NATIVE,
        AST_OP_HALT,

        AST_STRING,
        AST_NUMBER,
        AST_CHAR,
        AST_LABEL_DECL,
        AST_LABEL_CALL,
        AST_PROC,
    } tag;

    loc_t loc;
    union {
        struct ast_file {
            struct tasm_ast** lines; // label_decls, procs, instructions
            size_t line_size;
        } file;

        struct ast_inst {
            struct tasm_ast* operand; // number, char, label_calls
            const char* name;
        } inst;

        struct ast_label_decl {
            const char* name;
        } label_decl;

        struct ast_label_call {
            const char* name;
        } label_call;

        struct ast_char {
            const char* value;
        } character;

        struct ast_string {
            const char* value;
            size_t length;
        } string;

        struct ast_number {
            const char* text_value;
            union {
                float f32;
                unsigned int u32;
                int i32;
            } value;
        } number;

        struct ast_proc {
            const char* name;
            struct tasm_ast** lines; // labels, procs, instructions
            size_t line_size;
        } proc;

        // tile c interface
        struct ast_cfunction {
            uint8_t ret_type;
            uint8_t* arg_types;
            size_t arg_count;
            const char* name;
        } cfunction;

        struct ast_cstruct {
            const char* name;
        } cstruct;

        struct ast_data {
            struct tasm_ast* value;
        } data;
    };

} tasm_ast_t;

tasm_ast_t* tasm_ast_create(tasm_ast_t ast);
void tasm_ast_destroy(tasm_ast_t* node);
void tasm_ast_show(tasm_ast_t* root, int indent);

#ifdef TASM_AST_IMPLEMENTATION

#include <common/arena.h>
#include <stdio.h>

arena_t* ast_arena;

tasm_ast_t* tasm_ast_create(tasm_ast_t ast) {
    tasm_ast_t* ptr = arena_alloc(&ast_arena, sizeof(tasm_ast_t));
    if (ptr)
        *ptr = ast;
    return ptr;
}

void tasm_ast_destroy(tasm_ast_t* node) {
    if (!node) return;
    switch (node->tag) {
        case AST_FILE:
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_ast_destroy(node->file.lines[i]);
            }
            arrfree(node->file.lines);
            break;
        case AST_PROC:
            arrfree(node->proc.lines);
            break;
        case AST_CFUNCTION:
            arrfree(node->cfunction.arg_types);
            break;
        default:
            break;
    }
}

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void tasm_ast_show(tasm_ast_t* node, int indent) {
    if (!node) return;

    print_indent(indent);
    switch (node->tag) {
        case AST_NONE:
            printf("NONE\n");
            break;
        case AST_FILE:
            printf("FILE\n");
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_ast_show(node->file.lines[i], indent + 1);
            }
            break;
        case AST_OP_NOP:
            printf("NOP\n");
            break;
        case AST_OP_PUSH:
            printf("PUSH\n");
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_POP:
            printf("POP\n");
            break;
        case AST_OP_ADD:
            printf("ADD\n");
            break;
        case AST_OP_SUB:
            printf("SUB\n");
            break;
        case AST_OP_MULT:
            printf("MULT\n");
            break;
        case AST_OP_DIV:
            printf("DIV\n");
            break;
        case AST_OP_MOD:
            printf("MOD\n");
            break;
        case AST_OP_DUP:
            printf("DUP\n");
            break;
        case AST_OP_CLN:
            printf("CLN %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_SWAP:
            printf("SWAP %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_ADDF:
            printf("ADDF\n");
            break;
        case AST_OP_SUBF:
            printf("SUBF\n");
            break;
        case AST_OP_MULTF:
            printf("MULTF\n");
            break;
        case AST_OP_DIVF:
            printf("DIVF\n");
            break;
        case AST_OP_INC:
            printf("INC\n");
            break;
        case AST_OP_INCF:
            printf("INCF\n");
            break;
        case AST_OP_DEC:
            printf("DEC\n");
            break;
        case AST_OP_DECF:
            printf("DECF\n");
            break;
        case AST_OP_JMP:
            printf("JMP %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_JZ:
            printf("JZ %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_JNZ:
            printf("JNZ %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_CALL:
            printf("CALL %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_RET:
            printf("RET\n");
            break;
        case AST_OP_CI2F:
            printf("CI2F\n");
            break;
        case AST_OP_CI2U:
            printf("CI2U\n");
            break;
        case AST_OP_CF2I:
            printf("CF2I\n");
            break;
        case AST_OP_CF2U:
            printf("CF2U\n");
            break;
        case AST_OP_CU2I:
            printf("CU2I\n");
            break;
        case AST_OP_CU2F:
            printf("CU2F\n");
            break;
        case AST_OP_GT:
            printf("GT\n");
            break;
        case AST_OP_GTF:
            printf("GTF\n");
            break;
        case AST_OP_LT:
            printf("LT\n");
            break;
        case AST_OP_LTF:
            printf("LTF\n");
            break;
        case AST_OP_EQ:
            printf("EQ\n");
            break;
        case AST_OP_EQF:
            printf("EQF\n");
            break;
        case AST_OP_GE:
            printf("GE\n");
            break;
        case AST_OP_GEF:
            printf("GEF\n");
            break;
        case AST_OP_LE:
            printf("LE\n");
            break;
        case AST_OP_LEF:
            printf("LEF\n");
            break;
        case AST_OP_AND:
            printf("AND\n");
            break;
        case AST_OP_OR:
            printf("OR\n");
            break;
        case AST_OP_NOT:
            printf("NOT\n");
            break;
        case AST_OP_LOADC:
            printf("LOADC\n");
            break;
        case AST_OP_ALOADC:
            printf("ALOADC\n");
            break;
        case AST_OP_LOAD:
            printf("LOAD %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_STORE:
            printf("STORE %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_GLOAD:
            printf("GLOAD %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_GSTORE:
            printf("GSTORE %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_HALLOC:
            printf("HALLOC\n");
            break;
        case AST_OP_HSET:
            printf("HSET\n");
            break;
        case AST_OP_HSETOF:
            printf("HSETOF\n");
            break;
        case AST_OP_PUTS:
            printf("PUTS\n");
            break;
        case AST_OP_PUTC:
            printf("PUTC\n");
            break;
        case AST_OP_NATIVE:
            printf("NATIVE %s\n", node->inst.name);
            if (node->inst.operand) {
                tasm_ast_show(node->inst.operand, indent + 1);
            }
            break;
        case AST_OP_HALT:
            printf("HALT\n");
            break;
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
                tasm_ast_show(node->proc.lines[i], indent + 1);
            }
            break;

        case AST_DATA:
            printf("DATA\n");
            tasm_ast_show(node->data.value, indent + 1);
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
            printf("UNKNOWN\n");
            break;
    }
}

#endif//TASM_AST_IMPLEMENTATION

#endif//TASM_AST_H_
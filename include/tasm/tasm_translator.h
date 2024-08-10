#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include <common/arena.h>
#include <tasm/tasm_ast.h>
#include <tvm/tvm.h>

#define MAX_PROGRAM_CAPACITY 1024
#define SYMBOL_LABEL_CALL_CAPACITY 512
#define SYMBOL_LABEL_DECL_CAPACITY 512
#define SYMBOL_ARENA_CAPACITY 2048

typedef struct {
    struct {
        size_t addr;
        const char* name;
    } label_decls[SYMBOL_LABEL_DECL_CAPACITY];
    size_t label_decls_size;
    size_t label_address_pointer;
    struct {
        size_t addr;
        const char* name;
    } label_calls[SYMBOL_LABEL_CALL_CAPACITY];
    size_t label_calls_size;

} symbol_table_t;

typedef struct {
    symbol_table_t symbols;
    opcode_t program[MAX_PROGRAM_CAPACITY];
    size_t program_size;
    // arena_t symbol_arena;
} tasm_translator_t;

tasm_translator_t tasm_translator_init();
void tasm_translator_destroy(tasm_translator_t* translator);
void tasm_translate_unit(tasm_translator_t* translator, tasm_ast_t* node);
void tasm_resolve_labels(tasm_translator_t* translator, tasm_ast_t* node);
static void program_push(tasm_translator_t* translator, opcode_t code);
static size_t get_addr_from_label_decl_symbol(tasm_translator_t* translator, const char* name);
static size_t get_addr_from_label_call_symbol(tasm_translator_t* translator, const char* name);
void tasm_translator_generate_bin(tasm_translator_t* translator);
void symbol_dump(tasm_translator_t* translator);


#ifdef TASM_TRANSLATOR_IMPLEMENTATION

tasm_translator_t tasm_translator_init() {
    return (tasm_translator_t) {
        .program = {0},
        .symbols = (symbol_table_t){
            .label_calls = {0},
            .label_calls_size = 0,
            .label_decls = {0},
            .label_decls_size = 0,
            .label_address_pointer = 0,
        },
        .program_size = 0,
    };
}

void tasm_translator_destroy(tasm_translator_t* translator) {
    (void)translator;
}

void tasm_translate_unit(tasm_translator_t* translator, tasm_ast_t* node) {
   switch (node->tag) {
        case AST_NONE:
            break;
        case AST_FILE:
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_translate_unit(translator, node->file.lines[i]);
            }
            break;
        case AST_OP_NOP:
            program_push(translator, (opcode_t){.type = OP_NOP});
            break;
        case AST_OP_PUSH:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_PUSH,
                });
            }
            // TODO: implement errors
            break;
        case AST_OP_ADD:
            program_push(translator, (opcode_t){.type = OP_ADD});
            break;
        case AST_OP_SUB:
            program_push(translator, (opcode_t){.type = OP_SUB});
            break;
        case AST_OP_MULT:
            program_push(translator, (opcode_t){.type = OP_MULT});
            break;
        case AST_OP_DIV:
            program_push(translator, (opcode_t){.type = OP_DIV});
            break;
        case AST_OP_MOD:
            program_push(translator, (opcode_t){.type = OP_MOD});
            break;
        case AST_OP_DUP:
            program_push(translator, (opcode_t){.type = OP_DUP});
            break;
        case AST_OP_ADDF:
            program_push(translator, (opcode_t){.type = OP_ADDF});
            break;
        case AST_OP_SUBF:
            program_push(translator, (opcode_t){.type = OP_SUBF});
            break;
        case AST_OP_MULTF:
            program_push(translator, (opcode_t){.type = OP_MULTF});
            break;
        case AST_OP_DIVF:
            program_push(translator, (opcode_t){.type = OP_DIVF});
            break;
        case AST_OP_CALL:
            program_push(translator, (opcode_t){.type = OP_CALL});
            break;
        case AST_OP_RET:
            program_push(translator, (opcode_t){.type = OP_RET});
            break;
        case AST_OP_JMP:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_JMP,
                });
            }
            else if (node->inst.operand->tag == AST_LABEL_CALL) {
                tasm_translate_unit(translator, node->inst.operand);
                const char* name = node->inst.operand->label_call.name;
                size_t addr = get_addr_from_label_call_symbol(translator, name);
                if (addr == -1) {
                    fprintf(stderr, "There is no such a label called: %s\n", name);
                    return;
                }
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = addr,
                    .type = OP_JMP,
                });
            }
            break;
        case AST_OP_JNZ:
            break;
        case AST_OP_CSTF:
            break;
        case AST_OP_CSTI:
            break;
        case AST_OP_CSTU:
            break;
        case AST_OP_HALT:
            break;
        case AST_NUMBER:
            break;
        case AST_CHAR:
            break;
        case AST_LABEL_CALL: {
            const char* name = node->label_call.name;
            size_t addr = get_addr_from_label_decl_symbol(translator, name);
            printf("lbl cll addr: %d\n", addr);
            if (addr == -1) {
                fprintf(stderr, "There is no label declared before called: %s\n", name);
                return;
            }
            translator->symbols.label_calls[translator->symbols.label_calls_size].name = name;
            translator->symbols.label_calls[translator->symbols.label_calls_size].addr = addr;
            translator->symbols.label_calls_size++;
        }
            break;
        case AST_PROC:
            break;
        default:
            break;
    }
}

void tasm_resolve_labels(tasm_translator_t *translator, tasm_ast_t* node) {
    switch (node->tag) {
        case AST_NONE:
            break;
        case AST_FILE:
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_resolve_labels(translator, node->file.lines[i]);
            }
            break;
        case AST_OP_NOP:
        case AST_OP_PUSH:
        case AST_OP_ADD:
        case AST_OP_SUB:
        case AST_OP_MULT:
        case AST_OP_DIV:
        case AST_OP_MOD:
        case AST_OP_DUP:
        case AST_OP_ADDF:
        case AST_OP_SUBF:
        case AST_OP_MULTF:
        case AST_OP_DIVF:
        case AST_OP_CALL:
        case AST_OP_RET:
        case AST_OP_JMP:
        case AST_OP_JNZ:
        case AST_OP_CSTF:
        case AST_OP_CSTI:
        case AST_OP_CSTU:
        case AST_OP_HALT:
            translator->symbols.label_address_pointer++;
            break;
        case AST_LABEL_DECL: {
            // FIXME: check wheter label name already taken
            // TODO: hash tables or something can be used here as a performance improvement
            const char* name = node->label_decl.name;
            size_t addr = translator->symbols.label_address_pointer;
            translator->symbols.label_decls[translator->symbols.label_decls_size].name = name;
            translator->symbols.label_decls[translator->symbols.label_decls_size].addr = addr;
            translator->symbols.label_decls_size++;
            break;
        }
        case AST_PROC:
            break;
        default:
            break;
    }
}

static void program_push(tasm_translator_t* translator, opcode_t code) {
    translator->program[translator->program_size++] = code;
}

static size_t get_addr_from_label_decl_symbol(tasm_translator_t* translator, const char* name) {
    printf("gaflds: %d\n", translator->symbols.label_decls_size);
    for (size_t i = 0; i < translator->symbols.label_decls_size; i++) {
        if (strcmp(name, translator->symbols.label_decls[i].name) == 0) {
            return translator->symbols.label_decls[i].addr;
        }
    }
    return -1;
}

size_t get_addr_from_label_call_symbol(tasm_translator_t *translator, const char *name) {
    printf("gaflcs: %d\n", translator->symbols.label_calls_size);
    for (size_t i = 0; i < translator->symbols.label_calls_size; i++) {
        if (strcmp(name, translator->symbols.label_calls[i].name) == 0) {
            return translator->symbols.label_calls[i].addr;
        }
    }
    return -1;
}

void tasm_translator_generate_bin(tasm_translator_t *translator) {
    FILE* file;
    file = fopen("out.bin", "wb");

    fwrite(translator->program, sizeof(translator->program[0]), translator->program_size, file);

    fclose(file);
}

void symbol_dump(tasm_translator_t *translator) {
    printf("-------DECLS-------\n");
    printf("name, addr\n");
    for (size_t i = 0; i < translator->symbols.label_decls_size; i++) {
        printf("%s, %d\n", translator->symbols.label_decls[i].name, translator->symbols.label_decls[i].addr);
    }
    printf("-------CALLS-------\n");
    printf("name, addr\n");
    for (size_t i = 0; i < translator->symbols.label_calls_size; i++) {
        printf("%s, %d\n", translator->symbols.label_calls[i].name, translator->symbols.label_calls[i].addr);
    }
    
}

#endif//TRANSLATOR_IMPLEMENTATION

#endif//TRANSLATOR_H_
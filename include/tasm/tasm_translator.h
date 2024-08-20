#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include <common/arena.h>
#include <common/cmd_colors.h>
#include <tasm/tasm_ast.h>
#include <tvm/tvm.h>

#define MAX_PROGRAM_CAPACITY 1024
#define SYMBOL_LABEL_CALL_CAPACITY 512
#define SYMBOL_LABEL_DECL_CAPACITY 512
#define SYMBOL_PROC_DECL_CAPACITY 512

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
    struct {
        size_t addr;
        const char* name;
    } proc_decls[SYMBOL_PROC_DECL_CAPACITY];
    size_t proc_decls_size;
    size_t proc_address_pointer;

    bool err;

} symbol_table_t;

typedef struct {
    symbol_table_t symbols;
    opcode_t program[MAX_PROGRAM_CAPACITY];
    size_t program_size;
    arena_t cstr_arena;
} tasm_translator_t;

tasm_translator_t tasm_translator_init();
void tasm_translator_destroy(tasm_translator_t* translator);
static void tasm_translate_line(tasm_translator_t* translator, tasm_ast_t* node, const char* prefix, bool is_call);
static void tasm_translate_proc_and_line(tasm_translator_t* translator, tasm_ast_t* node);
void tasm_translate_unit(tasm_translator_t* translator, tasm_ast_t* node);
static void tasm_translate_proc(tasm_translator_t* translator, tasm_ast_t* node);
void tasm_resolve_labels(tasm_translator_t* translator, tasm_ast_t* node, const char* prefix);
void tasm_resolve_procs(tasm_translator_t* translator, tasm_ast_t* node);
static void program_push(tasm_translator_t* translator, opcode_t code);
static size_t get_addr_from_label_decl_symbol(tasm_translator_t* translator, const char* name);
static size_t get_addr_from_label_call_symbol(tasm_translator_t* translator, const char* name);
static size_t get_addr_from_proc_decl_symbol(tasm_translator_t* translator, const char* name);
void tasm_translator_generate_bin(tasm_translator_t* translator);
void symbol_dump(tasm_translator_t* translator);
bool is_err(tasm_translator_t* translator);


#ifdef TASM_TRANSLATOR_IMPLEMENTATION

tasm_translator_t tasm_translator_init() {
    return (tasm_translator_t) {
        .program = {0},
        .symbols = (symbol_table_t){
            .label_calls = {0},
            .label_calls_size = 0,
            .label_decls = {0},
            .label_decls_size = 0,
            .proc_decls = {0},
            .proc_decls_size = 0,
            .label_address_pointer = 0,
            .err = false,
        },
        .program_size = 0,
        .cstr_arena = arena_init(1024),
    };
}

void tasm_translator_destroy(tasm_translator_t* translator) {
    arena_destroy(&translator->cstr_arena);
}

static void tasm_translate_line(tasm_translator_t* translator, tasm_ast_t* node, const char* prefix, bool is_call) {
    switch (node->tag) {
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
            } else if (node->inst.operand->tag == AST_CHAR) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = (uint32_t)node->inst.operand->character.value[0],
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
        case AST_OP_CLN:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_CLN,
                });
            }
            break;
        case AST_OP_SWAP:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_SWAP,
                });
            }
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
        case AST_OP_INC:
            program_push(translator, (opcode_t){.type = OP_INC});
            break;
        case AST_OP_INCF:
            program_push(translator, (opcode_t){.type = OP_INCF});
            break;
        case AST_OP_DEC:
            program_push(translator, (opcode_t){.type = OP_DEC});
            break;
        case AST_OP_DECF:
            program_push(translator, (opcode_t){.type = OP_DECF});
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
                tasm_translate_line(translator, node->inst.operand, prefix, false);
                const char* name = node->inst.operand->label_call.name;
                int addr = get_addr_from_label_call_symbol(translator, name);
                if (addr == -1) {
                    return;
                }
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = addr,
                    .type = OP_JMP,
                });
            }
            break;
        case AST_OP_JZ:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_JZ,
                });
            }
            else if (node->inst.operand->tag == AST_LABEL_CALL) {
                tasm_translate_line(translator, node->inst.operand, prefix, false);
                const char* name = node->inst.operand->label_call.name;
                int addr = get_addr_from_label_call_symbol(translator, name);
                if (addr == -1) {
                    return;
                }
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = addr,
                    .type = OP_JZ,
                });
            }
            break;
        case AST_OP_JNZ:
            if (node->inst.operand->tag == AST_NUMBER) {
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = node->inst.operand->number.value.u32,
                    .type = OP_JNZ,
                });
            }
            else if (node->inst.operand->tag == AST_LABEL_CALL) {
                tasm_translate_line(translator, node->inst.operand, prefix, false);
                const char* name = node->inst.operand->label_call.name;
                int addr = get_addr_from_label_call_symbol(translator, name);
                if (addr == -1) {
                    return;
                }
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = addr,
                    .type = OP_JNZ,
                });
            }
            break;
        case AST_OP_CALL:
            if (node->inst.operand->tag == AST_LABEL_CALL) {
                tasm_translate_line(translator, node->inst.operand, NULL, true);
                const char* name = node->inst.operand->label_call.name;
                int addr = get_addr_from_proc_decl_symbol(translator, name);
                if (addr == -1) {
                    return;
                }
                program_push(translator, (opcode_t)
                {
                    .operand.ui32 = addr,
                    .type = OP_CALL,
                });
            }
            break;
        case AST_OP_RET:
            program_push(translator, (opcode_t){.type = OP_RET});
            break;
        case AST_OP_CI2F:
            program_push(translator, (opcode_t){.type = OP_CI2F});
            break;
        case AST_OP_CI2U:
            program_push(translator, (opcode_t){.type = OP_CI2U});
            break;
        case AST_OP_CF2I:
            program_push(translator, (opcode_t){.type = OP_CF2I});
            break;
        case AST_OP_CF2U:
            program_push(translator, (opcode_t){.type = OP_CF2U});
            break;
        case AST_OP_CU2I:
            program_push(translator, (opcode_t){.type = OP_CU2I});
            break;
        case AST_OP_CU2F:
            program_push(translator, (opcode_t){.type = OP_CU2F});
            break;
        case AST_OP_HALT:
            program_push(translator, (opcode_t){.type = OP_HALT});
            break;
        case AST_NUMBER:
            break;
        case AST_CHAR:
            break;
        case AST_LABEL_CALL: {
            char* name = NULL;
            int addr;
            if (prefix != NULL) {
                size_t size2 = strlen(prefix);
                size_t size1 = strlen(node->label_call.name);
                name = arena_alloc(&translator->cstr_arena, size1 + size2 + 2);
                strcpy(name, prefix);
                strcat(name, "$");
                strcat(name, node->label_call.name);
                node->label_call.name = name;
                addr = get_addr_from_label_decl_symbol(translator, name);
            } else {
                name = (char*)node->label_call.name;
                if (is_call)
                    addr = get_addr_from_proc_decl_symbol(translator, name);
                else
                    addr = get_addr_from_label_decl_symbol(translator, name);
            }

            if (addr == -1) {
                fprintf(stderr, "%s:%d:%d:"CLR_RED"Unresolved symbol:"CLR_END" %s\n", node->loc.file_name, node->loc.row, node->loc.col, name);
                translator->symbols.err = true;
                return;
            }
            translator->symbols.label_calls[translator->symbols.label_calls_size].name = name;
            translator->symbols.label_calls[translator->symbols.label_calls_size].addr = addr;
            translator->symbols.label_calls_size++;
        }
            break;
        case AST_PROC:
            fprintf(stderr, CLR_RED"It is not possible to create proc inside another proc!"CLR_END);
            break;
        default:
            fprintf(stderr, "");
            break;
    }
}

static void tasm_translate_proc_and_line(tasm_translator_t *translator, tasm_ast_t *node) {
    switch (node->tag)
    {
    case AST_PROC:
        tasm_translate_proc(translator, node);
        break;
    default:
        tasm_translate_line(translator, node, NULL, false);
        break;
    }
}

void tasm_translate_unit(tasm_translator_t *translator, tasm_ast_t *node) {
    switch (node->tag)
    {
    case AST_NONE:
        break;
    case AST_FILE:
        for (size_t i = 0; i < node->file.line_size; i++) {
            tasm_translate_proc_and_line(translator, node->file.lines[i]);
        }
        break;
    default:
        break;
    }
}

static void tasm_translate_proc(tasm_translator_t *translator, tasm_ast_t *node) {
    switch (node->tag)
    {
    case AST_NONE:
        break;
    case AST_PROC:
        for (size_t i = 0; i < node->proc.line_size; i++) {
            tasm_translate_line(translator, node->proc.lines[i], node->proc.name, false);
        }
        break;
    default:
        break;
    }
}

void tasm_resolve_labels(tasm_translator_t *translator, tasm_ast_t* node, const char* prefix) {
    switch (node->tag) {
        case AST_NONE:
            break;
        case AST_FILE:
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_resolve_labels(translator, node->file.lines[i], NULL);
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
        case AST_OP_CLN:
        case AST_OP_SWAP:
        case AST_OP_ADDF:
        case AST_OP_SUBF:
        case AST_OP_MULTF:
        case AST_OP_DIVF:
        case AST_OP_INC:
        case AST_OP_INCF:
        case AST_OP_DEC:
        case AST_OP_DECF:
        case AST_OP_JMP:
        case AST_OP_JZ:
        case AST_OP_JNZ:
        case AST_OP_CALL:
        case AST_OP_RET:
        case AST_OP_CI2F:
        case AST_OP_CI2U:
        case AST_OP_CF2I:
        case AST_OP_CF2U:
        case AST_OP_CU2I:
        case AST_OP_CU2F:
        case AST_OP_HALT:
            translator->symbols.label_address_pointer++;
            break;
        case AST_LABEL_DECL: {
            // FIXME: support multiple erro messages
            // TODO: hash tables or something can be used here as a performance improvement
            char* name = NULL;
            if (prefix != NULL) {
                size_t size2 = strlen(prefix);
                size_t size1 = strlen(node->label_decl.name);
                name = arena_alloc(&translator->cstr_arena, size1 + size2 + 2);
                strcpy(name, prefix);
                strcat(name, "$");
                strcat(name, node->label_decl.name);
            } else {
                name = (char*)node->label_decl.name;
            }
            for (size_t i = 0; i < translator->symbols.label_decls_size; i++) {
                if (strcmp(translator->symbols.label_decls[i].name, name) == 0) {
                    fprintf(stderr, "%s:%d:%d:"CLR_RED"Duplicated label decleration:"CLR_END" %s\n", node->loc.file_name, node->loc.row, node->loc.col, name);
                    translator->symbols.err = true;
                    exit(1);
                }
            }
            size_t addr = translator->symbols.label_address_pointer;
            translator->symbols.label_decls[translator->symbols.label_decls_size].name = name;
            translator->symbols.label_decls[translator->symbols.label_decls_size].addr = addr;
            translator->symbols.label_decls_size++;
            break;
        }
        case AST_PROC:
            for (size_t i = 0; i < node->proc.line_size; i++) {
                tasm_resolve_labels(translator, node->proc.lines[i], node->proc.name);
            }
            break;
        default:
            break;
    }
}

void tasm_resolve_procs(tasm_translator_t *translator, tasm_ast_t *node) {
    switch (node->tag) {
        case AST_NONE:
            break;
        case AST_FILE:
            for (size_t i = 0; i < node->file.line_size; i++) {
                tasm_resolve_procs(translator, node->file.lines[i]);
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
        case AST_OP_CLN:
        case AST_OP_SWAP:
        case AST_OP_ADDF:
        case AST_OP_SUBF:
        case AST_OP_MULTF:
        case AST_OP_DIVF:
        case AST_OP_INC:
        case AST_OP_INCF:
        case AST_OP_DEC:
        case AST_OP_DECF:
        case AST_OP_JMP:
        case AST_OP_JZ:
        case AST_OP_JNZ:
        case AST_OP_CALL:
        case AST_OP_RET:
        case AST_OP_CI2F:
        case AST_OP_CI2U:
        case AST_OP_CF2I:
        case AST_OP_CF2U:
        case AST_OP_CU2I:
        case AST_OP_CU2F:
        case AST_OP_HALT:
            translator->symbols.proc_address_pointer++;
            break;
        case AST_PROC: {
            // FIXME: support multiple erro messages
            // TODO: hash tables or something can be used here as a performance improvement
            const char* name = node->proc.name;
            size_t addr = translator->symbols.proc_address_pointer;
            for (size_t i = 0; i < translator->symbols.proc_decls_size; i++) {
                if (strcmp(translator->symbols.proc_decls[i].name, name) == 0) {
                    fprintf(stderr, "%s:%d:%d:"CLR_RED"Duplicated proc decleration:"CLR_END" %s\n", node->loc.file_name, node->loc.row, node->loc.col, name);
                    translator->symbols.err = true;
                    exit(1);
                }
            }
            translator->symbols.proc_decls[translator->symbols.proc_decls_size].name = name;
            translator->symbols.proc_decls[translator->symbols.proc_decls_size].addr = addr;
            translator->symbols.proc_decls_size++;

            for (size_t i = 0; i < node->proc.line_size; i++) {
                tasm_resolve_procs(translator, node->proc.lines[i]);
            }
            break;
        }
        default:
            break;
    }
}

static void program_push(tasm_translator_t* translator, opcode_t code) {
    translator->program[translator->program_size++] = code;
}

static size_t get_addr_from_label_decl_symbol(tasm_translator_t* translator, const char* name) {
    for (size_t i = 0; i < translator->symbols.label_decls_size; i++) {
        if (strcmp(name, translator->symbols.label_decls[i].name) == 0) {
            return translator->symbols.label_decls[i].addr;
        }
    }
    return -1;
}

size_t get_addr_from_label_call_symbol(tasm_translator_t *translator, const char *name) {
    for (size_t i = 0; i < translator->symbols.label_calls_size; i++) {
        if (strcmp(name, translator->symbols.label_calls[i].name) == 0) {
            return translator->symbols.label_calls[i].addr;
        }
    }
    return -1;
}

size_t get_addr_from_proc_decl_symbol(tasm_translator_t *translator, const char *name) {
    for (size_t i = 0; i < translator->symbols.proc_decls_size; i++) {
        if (strcmp(name, translator->symbols.proc_decls[i].name) == 0) {
            return translator->symbols.proc_decls[i].addr;
        }
    }
    return -1;
}

void tasm_translator_generate_bin(tasm_translator_t *translator) {
    FILE* file;
    file = fopen("out.bin", "wb");

    fwrite(translator->program, sizeof(translator->program[0]), translator->program_size, file);

    fclose(file);
    fprintf(stdout, "out.bin created "CLR_GREEN"successfully."CLR_END"\n");
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
    printf("-------PROC-DECLS-------\n");
    printf("name, addr\n");
    for (size_t i = 0; i < translator->symbols.proc_decls_size; i++) {
        printf("%s, %d\n", translator->symbols.proc_decls[i].name, translator->symbols.proc_decls[i].addr);
    }
    
}

bool is_err(tasm_translator_t* translator) {
    return translator->symbols.err;
}

#endif//TRANSLATOR_IMPLEMENTATION

#endif//TRANSLATOR_H_
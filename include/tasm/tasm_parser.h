#ifndef TASM_PARSER_H_
#define TASM_PARSER_H_

#include <tasm/tasm_lexer.h>
#include <tasm/tasm_ast.h>
#include <common/cmd_colors.h>

/*
    this err codes are between 1000 - 10000
    * first digit shows err type
    * second digit shows different version of that erro type
    * third digit shows nothing for now
    * fourth digit shows where that erro can be happened
        - 0: global scope
        - 1: proc scope
        - 2: both scopes
*/
#define COMPOSITE_TYPE_ERR_CODE 1000
#define COMPSITE_ERR_FILE_LINE_UNEXPECTED_ID_OR_NUMBER 1200
#define COMPSITE_ERR_PROC_LINE_UNEXPECTED_ID_OR_NUMBER 1201
#define COMPSITE_ERR_PUSH_WRONG_OPERAND                2002
#define COMPSITE_ERR_CLN_WRONG_OPERAND                 2102
#define COMPSITE_ERR_SWAP_WRONG_OPERAND                2202
#define COMPSITE_ERR_JMP_WRONG_OPERAND                 2302
#define COMPSITE_ERR_JZ_WRONG_OPERAND                  2402
#define COMPSITE_ERR_JNZ_WRONG_OPERAND                 2502
#define COMPSITE_ERR_CALL_WRONG_OPERAND                2602
#define COMPSITE_ERR_NATIVE_WRONG_OPERAND              2702
#define COMPSITE_ERR_PROC_INSIDE_PROC                  3401
#define COMPSITE_ERR_CINTERFACE_INSIDE_PROC            3601
#define COMPSITE_ERR_CINTERFACE_RET_TYPE_ERR           4000
#define COMPSITE_ERR_CINTERFACE_ARG_TYPE_ERR           4100
#define COMPSITE_ERR_FILE_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE  5100
#define COMPSITE_ERR_PROC_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE  5101

typedef struct {
    tasm_token_t current_token;
    tasm_token_t prev_token;
    tasm_token_t next_token;
    tasm_lexer_t* lexer;
} tasm_parser_t;

tasm_parser_t tasm_parser_init(tasm_lexer_t* lexer);
void tasm_parser_destroy(tasm_parser_t* parser);

void tasm_parser_eat(tasm_parser_t* parser, token_type_t token_type);

tasm_ast_t* tasm_parse_file(tasm_parser_t *parser);
tasm_ast_t* tasm_parse_line(tasm_parser_t *parser);
tasm_ast_t* tasm_parse_proc_line(tasm_parser_t* parser);

tasm_ast_t* tasm_parse_instruction(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_label_decl(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_proc(tasm_parser_t* parser);

tasm_ast_t* tasm_parse_metadata(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_cfunction(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_cstruct(tasm_parser_t* parser);

tasm_ast_t* tasm_parse_number_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_int_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_jmp_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_label_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_push_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_label_call(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_char_lit(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_num_lit(tasm_parser_t* parser, bool negative);

bool is_operand_number(tasm_parser_t* parser);
bool is_operand_char(tasm_parser_t* parser);
bool is_operand_label_call(tasm_parser_t* parser);

#ifdef TASM_PARSER_IMPLEMENTATION

#include <math.h>
#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>
#include <stdint.h>

tasm_parser_t tasm_parser_init(tasm_lexer_t* lexer) {
    tasm_parser_t parser = {
        .lexer = lexer,
        .current_token = tasm_token_create(TOKEN_NONE, NULL),
        .prev_token = tasm_token_create(TOKEN_NONE, NULL),
        .next_token = tasm_lexer_get_next_token(lexer),
    };
    return parser;
}

void tasm_parser_destroy(tasm_parser_t* parser) {
    tasm_lexer_destroy(parser->lexer);
}

void tasm_parser_eat_err_msg(int token_type) {
    if (token_type < COMPOSITE_TYPE_ERR_CODE)
        return;
    switch (token_type)
    {
    case COMPSITE_ERR_FILE_LINE_UNEXPECTED_ID_OR_NUMBER:
        fprintf(stderr, "COMPSITE_ERR_FILE_LINE_UNEXPECTED_ID_OR_NUMBER\n");
        break;
    case COMPSITE_ERR_PROC_LINE_UNEXPECTED_ID_OR_NUMBER:
        fprintf(stderr, "COMPSITE_ERR_PROC_LINE_UNEXPECTED_ID_OR_NUMBER\n");
        break;
    case COMPSITE_ERR_PUSH_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_PUSH_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_CLN_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_CLN_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_SWAP_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_SWAP_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_JMP_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_JMP_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_JZ_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_JZ_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_JNZ_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_JNZ_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_CALL_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_CALL_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_NATIVE_WRONG_OPERAND:
        fprintf(stderr, "COMPSITE_ERR_NATIVE_WRONG_OPERAND\n");
        break;
    case COMPSITE_ERR_PROC_INSIDE_PROC:
        fprintf(stderr, "COMPSITE_ERR_PROC_INSIDE_PROC\n");
        break;
    case COMPSITE_ERR_CINTERFACE_INSIDE_PROC:
        fprintf(stderr, "COMPSITE_ERR_CINTERFACE_INSIDE_PROC\n");
        break;
    case COMPSITE_ERR_CINTERFACE_RET_TYPE_ERR:
        fprintf(stderr, "COMPSITE_ERR_CINTERFACE_RET_TYPE_ERR\n");
        break;
    case COMPSITE_ERR_CINTERFACE_ARG_TYPE_ERR:
        fprintf(stderr, "COMPSITE_ERR_CINTERFACE_ARG_TYPE_ERR\n");
        break;
    case COMPSITE_ERR_FILE_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE:
        fprintf(stderr, "COMPSITE_ERR_FILE_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE\n");
        break;
    case COMPSITE_ERR_PROC_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE:
        fprintf(stderr, "COMPSITE_ERR_PROC_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE\n");
        break;
    default:
        fprintf(stderr, "NOT IMPLEMENTED ERR TYPE\n");
        break;
    }
}

void tasm_parser_eat(tasm_parser_t* parser, token_type_t token_type) {
    if (parser->current_token.type != token_type) {
        printf(
        "%s:%d:%d:"CLR_RED"Unexpected token:"CLR_END"`%s`, with type `%d`\nExpected type `%d`\n",
        parser->lexer->loc.file_name,
        parser->lexer->loc.row,
        parser->lexer->loc.col,
        parser->current_token.value,
        parser->current_token.type,
        token_type
        );
        tasm_lexer_destroy(parser->lexer);
        tasm_parser_eat_err_msg(token_type);
        exit(-1);
    } else {
        parser->prev_token = parser->current_token;
        parser->current_token = parser->next_token;
        parser->next_token = tasm_lexer_get_next_token(parser->lexer);
    }
}

tasm_ast_t* tasm_parse_file(tasm_parser_t* parser) {
    tasm_parser_eat(parser, TOKEN_NONE);
    tasm_ast_t** lines = NULL;
    while (parser->current_token.type != TOKEN_EOF) {
        tasm_ast_t* line = tasm_parse_line(parser);
        if (line != NULL)
            arrput(lines, line);
    }
    tasm_ast_t* ast_file = tasm_ast_create((tasm_ast_t) {
        .tag = AST_FILE,
        .loc = parser->lexer->loc,
        .file.lines = lines,
        .file.line_size = arrlen(lines),
    });
    return ast_file;
}

bool is_line_label_decl(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_ID
    && parser->next_token.type == TOKEN_COLON)
        return true;
    return false;
}

bool is_line_proc(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_PROC)
        return true;
    return false;
}

bool is_line_endp(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_ENDP)
        return true;
    return false;
}

bool is_line_instruction(tasm_parser_t* parser) {
    if (parser->current_token.type > INSTRUCTIONS_TOKEN_BEGIN
    && parser->current_token.type < INSTRUCTIONS_TOKEN_END)
        return true;
    return false;
}

bool is_line_cinterface(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_AT)
        return true;
    return false;
}

bool is_line_cinterface_token(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_CFUNCTION
    ||  parser->current_token.type == TOKEN_CSTRUCT
    ||  (parser->current_token.type > TOKEN_TCI_BEGIN && parser->current_token.type < TOKEN_TCI_END))
        return true;
    return false;
}

bool is_token_ctype(tasm_parser_t* parser) {
    if (parser->current_token.type > TOKEN_TCI_BEGIN
    && parser->current_token.type < TOKEN_TCI_END)
        return true;
    return false;
}

tasm_ast_t* tasm_parse_line(tasm_parser_t* parser) {
    if (is_line_label_decl(parser))
        return tasm_parse_label_decl(parser);
    if (is_line_proc(parser))
        return tasm_parse_proc(parser);
    if (is_line_instruction(parser))
        return tasm_parse_instruction(parser);
    if (is_line_cinterface(parser))
        return tasm_parse_metadata(parser);

    if (parser->current_token.type == TOKEN_ID ||
        parser->current_token.type == TOKEN_DECIMAL_NUMBER ||
        parser->current_token.type == TOKEN_HEX_NUMBER ||
        parser->current_token.type == TOKEN_FLOAT_NUMBER ||
        parser->current_token.type == TOKEN_BINARY_NUMBER)
        tasm_parser_eat(parser, COMPSITE_ERR_FILE_LINE_UNEXPECTED_ID_OR_NUMBER);

    if (is_line_cinterface_token(parser))
        tasm_parser_eat(parser, COMPSITE_ERR_FILE_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE);

    if (parser->current_token.type == TOKEN_ENDLINE)
        tasm_parser_eat(parser, TOKEN_ENDLINE);
    if (parser->current_token.type == TOKEN_COMMENT)
        tasm_parser_eat(parser, TOKEN_COMMENT);
    if (parser->current_token.type == TOKEN_EOF)
        tasm_parser_eat(parser, TOKEN_EOF);

    return NULL;
}

tasm_ast_t* tasm_parse_proc_line(tasm_parser_t* parser) {
    if (is_line_label_decl(parser))
        return tasm_parse_label_decl(parser);
    if (is_line_instruction(parser))
        return tasm_parse_instruction(parser);
    if (is_line_proc(parser))
        tasm_parser_eat(parser, COMPSITE_ERR_PROC_INSIDE_PROC);
    if (is_line_cinterface(parser))
        tasm_parser_eat(parser, COMPSITE_ERR_CINTERFACE_INSIDE_PROC);

    if (parser->current_token.type == TOKEN_ID ||
        parser->current_token.type == TOKEN_DECIMAL_NUMBER ||
        parser->current_token.type == TOKEN_HEX_NUMBER ||
        parser->current_token.type == TOKEN_FLOAT_NUMBER ||
        parser->current_token.type == TOKEN_BINARY_NUMBER)
        tasm_parser_eat(parser, COMPSITE_ERR_PROC_LINE_UNEXPECTED_ID_OR_NUMBER);

    if (is_line_cinterface_token(parser))
        tasm_parser_eat(parser, COMPSITE_ERR_PROC_LINE_UNEXPECTED_CINTERFACE_TOKEN_TYPE);

    if (parser->current_token.type == TOKEN_ENDLINE)
        tasm_parser_eat(parser, TOKEN_ENDLINE);
    if (parser->current_token.type == TOKEN_COMMENT)
        tasm_parser_eat(parser, TOKEN_COMMENT);
    if (parser->current_token.type == TOKEN_EOF)
        tasm_parser_eat(parser, TOKEN_EOF);

    return NULL;
}

tasm_ast_t* tasm_parse_label_decl(tasm_parser_t* parser) {
    const char* label_name = parser->current_token.value;
    const loc_t loc = parser->lexer->loc;
    tasm_parser_eat(parser, TOKEN_ID);
    tasm_parser_eat(parser, TOKEN_COLON);
    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_LABEL_DECL,
        .loc = loc,
        .label_decl.name = label_name,
    });
}

tasm_ast_t* tasm_parse_proc(tasm_parser_t *parser) {
    tasm_parser_eat(parser, TOKEN_PROC);
    const char* proc_name = parser->current_token.value;
    const loc_t loc = parser->lexer->loc;
    tasm_parser_eat(parser, TOKEN_ID);
    
    tasm_ast_t** lines = NULL;
    while (parser->current_token.type != TOKEN_ENDP) {
        tasm_ast_t* line = tasm_parse_proc_line(parser);
        if (line != NULL)
            arrput(lines, line);
        if (parser->current_token.type == TOKEN_EOF)
            tasm_parser_eat(parser, TOKEN_ENDP);
    }
    tasm_parser_eat(parser, TOKEN_ENDP);
    tasm_ast_t* ast_proc = tasm_ast_create((tasm_ast_t) {
        .tag = AST_PROC,
        .loc = loc,
        .proc.name = proc_name,
        .proc.lines = lines,
        .proc.line_size = arrlen(lines),
    });

    if (parser->current_token.type == TOKEN_COMMENT)
        tasm_parser_eat(parser, TOKEN_COMMENT);
    if (parser->current_token.type == TOKEN_EOF)
        tasm_parser_eat(parser, TOKEN_EOF);
    else
        tasm_parser_eat(parser, TOKEN_ENDLINE);

    return ast_proc;
}

bool is_operand_number(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_DECIMAL_NUMBER || parser->current_token.type == TOKEN_FLOAT_NUMBER || parser->current_token.type == TOKEN_HEX_NUMBER ||parser->current_token.type == TOKEN_BINARY_NUMBER)
        return true;
    return false;
}

bool is_operand_int(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_DECIMAL_NUMBER || parser->current_token.type == TOKEN_HEX_NUMBER)
        return true;
    return false;
}

bool is_operand_char(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_APOST)
        return true;
    return false;
}

bool is_operand_label_call(tasm_parser_t* parser) {
    if (parser->current_token.type == TOKEN_ID)
        return true;
    return false;
}


tasm_ast_t* tasm_parse_instruction(tasm_parser_t* parser) {
    const char* name = parser->current_token.value;
    int type = parser->current_token.type;
    const loc_t loc = parser->lexer->loc;
    tasm_parser_eat(parser, parser->current_token.type);
        
    tasm_ast_t* operand = NULL;
    int tag;
    switch (type) {
    case TOKEN_OP_NOP: tag = AST_OP_NOP;
        break;
    case TOKEN_OP_PUSH: tag = AST_OP_PUSH;
        operand = tasm_parse_push_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_PUSH_WRONG_OPERAND);
        break;
    case TOKEN_OP_POP: tag = AST_OP_POP;
        break;
    case TOKEN_OP_ADD: tag = AST_OP_ADD;
        break;
    case TOKEN_OP_SUB: tag = AST_OP_SUB;
        break;
    case TOKEN_OP_MULT: tag = AST_OP_MULT;
        break;
    case TOKEN_OP_DIV: tag = AST_OP_DIV;
        break;
    case TOKEN_OP_MOD: tag = AST_OP_MOD;
        break;
    case TOKEN_OP_DUP: tag = AST_OP_DUP;
        break;
    case TOKEN_OP_CLN: tag = AST_OP_CLN;
        operand = tasm_parse_int_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_CLN_WRONG_OPERAND);
        break;
    case TOKEN_OP_SWAP: tag = AST_OP_SWAP;
        operand = tasm_parse_int_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_SWAP_WRONG_OPERAND);
        break;
    case TOKEN_OP_ADDF: tag = AST_OP_ADDF;
        break;
    case TOKEN_OP_SUBF: tag = AST_OP_SUBF;
        break;
    case TOKEN_OP_MULTF: tag = AST_OP_MULTF;
        break;
    case TOKEN_OP_DIVF: tag = AST_OP_DIVF;
        break;
    case TOKEN_OP_INC: tag = AST_OP_INC;
        break;
    case TOKEN_OP_DEC: tag = AST_OP_DEC;
        break;
    case TOKEN_OP_INCF: tag = AST_OP_INCF;
        break;
    case TOKEN_OP_DECF: tag = AST_OP_DECF;
        break;
    case TOKEN_OP_JMP: tag = AST_OP_JMP;
        operand = tasm_parse_jmp_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_JMP_WRONG_OPERAND);
        break;
    case TOKEN_OP_JZ: tag = AST_OP_JZ;
        operand = tasm_parse_jmp_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_JZ_WRONG_OPERAND);
        break;
    case TOKEN_OP_JNZ: tag = AST_OP_JNZ;
        operand = tasm_parse_jmp_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_JNZ_WRONG_OPERAND);
        break;
    case TOKEN_OP_CALL: tag = AST_OP_CALL;
        operand = tasm_parse_label_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_CALL_WRONG_OPERAND);
        break;
    case TOKEN_OP_RET: tag = AST_OP_RET;
        break;
    case TOKEN_OP_CI2F: tag = AST_OP_CI2F;
        break;
    case TOKEN_OP_CI2U: tag = AST_OP_CI2U;
        break;
    case TOKEN_OP_CF2I: tag = AST_OP_CF2I;
        break;
    case TOKEN_OP_CF2U: tag = AST_OP_CF2U;
        break;
    case TOKEN_OP_CU2I: tag = AST_OP_CU2I;
        break;
    case TOKEN_OP_CU2F: tag = AST_OP_CU2F;
        break;
    case TOKEN_OP_GT: tag = AST_OP_GT;
        break;
    case TOKEN_OP_GTF: tag = AST_OP_GTF;
        break;
    case TOKEN_OP_LT: tag = AST_OP_LT;
        break;
    case TOKEN_OP_LTF: tag = AST_OP_LTF;
        break;
    case TOKEN_OP_EQ: tag = AST_OP_EQ;
        break;
    case TOKEN_OP_EQF: tag = AST_OP_EQF;
        break;
    case TOKEN_OP_GE: tag = AST_OP_GE;
        break;
    case TOKEN_OP_GEF: tag = AST_OP_GEF;
        break;
    case TOKEN_OP_LE: tag = AST_OP_LE;
        break;
    case TOKEN_OP_LEF: tag = AST_OP_LEF;
        break;
    case TOKEN_OP_NATIVE: tag = AST_OP_NATIVE;
        operand = tasm_parse_int_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, COMPSITE_ERR_NATIVE_WRONG_OPERAND);
        break;
    case TOKEN_OP_HALT: tag = AST_OP_HALT;
        break;
    default:
        break;
    }

    if (parser->current_token.type == TOKEN_COMMENT)
        tasm_parser_eat(parser, TOKEN_COMMENT);
    if (parser->current_token.type == TOKEN_EOF)
        tasm_parser_eat(parser, TOKEN_EOF);
    else
        tasm_parser_eat(parser, TOKEN_ENDLINE);

    return tasm_ast_create((tasm_ast_t) {
        .tag = tag,
        .loc = loc,
        .inst.name = name,
        .inst.operand = operand,
    });
}

tasm_ast_t* tasm_parse_metadata(tasm_parser_t* parser) {
    tasm_parser_eat(parser, TOKEN_AT);
    if (parser->current_token.type == TOKEN_CFUNCTION)
        return tasm_parse_cfunction(parser);
    if (parser->current_token.type == TOKEN_CSTRUCT)
        return tasm_parse_cstruct(parser);
    
    tasm_parser_eat(parser, 8000);
    return NULL;
}

tasm_ast_t* tasm_parse_cfunction(tasm_parser_t* parser) {
    tasm_parser_eat(parser, TOKEN_CFUNCTION);
    const loc_t loc = parser->lexer->loc;

    int rtype = parser->current_token.type - TOKEN_TCI_BEGIN - 1;
    if (!is_token_ctype(parser))
        tasm_parser_eat(parser, COMPSITE_ERR_CINTERFACE_RET_TYPE_ERR);
    tasm_parser_eat(parser, parser->current_token.type);

    const char* fun_name = parser->current_token.value;
    tasm_parser_eat(parser, TOKEN_ID);

    uint16_t argcount = 0;
    uint8_t* argtpyes = NULL;
    for (; parser->current_token.type != TOKEN_ENDLINE && parser->current_token.type != TOKEN_EOF && parser->current_token.type != TOKEN_COMMENT;) {
        if (!is_token_ctype(parser))
            tasm_parser_eat(parser, COMPSITE_ERR_CINTERFACE_ARG_TYPE_ERR);
        arrput(argtpyes, parser->current_token.type - TOKEN_TCI_BEGIN - 1);
        tasm_parser_eat(parser, parser->current_token.type);
        argcount++;
    }
    
    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_CFUNCTION,
        .loc = loc,
        .cfunction.ret_type = rtype,
        .cfunction.arg_types = argtpyes,
        .cfunction.arg_count = argcount,
        .cfunction.name = fun_name,
    });
}

tasm_ast_t* tasm_parse_cstruct(tasm_parser_t* parser) {
    tasm_parser_eat(parser, TOKEN_CSTRUCT);
    const loc_t loc = parser->lexer->loc;

    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_CFUNCTION,
        .loc = loc,
        .cstruct.name = "cstruct",
    });
}


tasm_ast_t* tasm_parse_number_operand(tasm_parser_t *parser) {
    bool negative = false;
    if (parser->current_token.type == TOKEN_MINUS) {
        tasm_parser_eat(parser, TOKEN_MINUS);
        negative = true;
    }
    if (parser->current_token.type == TOKEN_DECIMAL_NUMBER ||
        parser->current_token.type == TOKEN_FLOAT_NUMBER ||
        parser->current_token.type == TOKEN_HEX_NUMBER ||
        parser->current_token.type == TOKEN_BINARY_NUMBER) {
        return tasm_parse_num_lit(parser, negative);
    }
    return NULL;
}

tasm_ast_t* tasm_parse_int_operand(tasm_parser_t *parser) {
    if (is_operand_int(parser))
        return tasm_parse_num_lit(parser, false);
    return NULL;
}

tasm_ast_t* tasm_parse_jmp_operand(tasm_parser_t* parser) {
    if (is_operand_int(parser))
        return tasm_parse_num_lit(parser, false);
    if (is_operand_label_call(parser))
        return tasm_parse_label_call(parser);

    return NULL;
}

tasm_ast_t* tasm_parse_label_operand(tasm_parser_t *parser) {
    if (is_operand_label_call(parser))
        return tasm_parse_label_call(parser);
    return NULL;
}

tasm_ast_t* tasm_parse_push_operand(tasm_parser_t* parser) {
    if (is_operand_char(parser))
        return tasm_parse_char_lit(parser);
    
    return tasm_parse_number_operand(parser);

    return NULL;
}


tasm_ast_t* tasm_parse_label_call(tasm_parser_t *parser) {
    const char* name = parser->current_token.value;
    const loc_t loc = parser->lexer->loc;
    tasm_parser_eat(parser, TOKEN_ID);
    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_LABEL_CALL,
        .loc = loc,
        .label_call.name = name,
    });
}

tasm_ast_t* tasm_parse_num_lit(tasm_parser_t* parser, bool negative) {
    char* text_val = parser->current_token.value;
    token_type_t type = parser->current_token.type;
    int32_t value;
    
    if (negative) {
        size_t size = strlen(parser->current_token.value);
        text_val = arena_alloc(parser->lexer->tokens_arena, size + 2);
        memmove(&text_val[1], parser->current_token.value, size);
        text_val[0] = '-';
        text_val[size + 1] = '\0';
    }

    switch (type)
    {
    case TOKEN_DECIMAL_NUMBER: {
        int val = atoi(text_val);
        value = *(uint32_t*)&val;
        break;
    }
    case TOKEN_FLOAT_NUMBER: {
        float val = strtof(text_val, NULL);
        value = *(uint32_t*)&val;
        break;
    }
    case TOKEN_HEX_NUMBER: {
        int val = strtol(text_val, NULL, 16);
        value = *(uint32_t*)&val;
        break;
    }
    case TOKEN_BINARY_NUMBER: {
        int val = strtol(text_val, NULL, 2);
        value = *(uint32_t*)&val;
        break;
    }
    
    default:
        tasm_parser_eat(parser, 9600);
        break;
    }
    tasm_parser_eat(parser, type);
    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_NUMBER,
        .loc = parser->lexer->loc,
        .number.text_value = text_val,
        .number.value.i32 = value,
    });
}

tasm_ast_t* tasm_parse_char_lit(tasm_parser_t* parser) {
    tasm_parser_eat(parser, TOKEN_APOST);
    const char* val = parser->current_token.value;
    tasm_parser_eat(parser, TOKEN_ID);
    tasm_parser_eat(parser, TOKEN_APOST);
    return tasm_ast_create((tasm_ast_t) {
        .tag = AST_CHAR,
        .loc = parser->lexer->loc,
        .character.value = val,
    });
}


#endif//TASM_PARSER_IMPLEMENTATION

#endif//TASM_PARSER_H_
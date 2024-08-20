#ifndef TASM_PARSER_H_
#define TASM_PARSER_H_

#include <tasm/tasm_lexer.h>
#include <tasm/tasm_ast.h>
#include <common/cmd_colors.h>

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


tasm_ast_t* tasm_parse_number_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_int_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_jmp_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_label_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_push_operand(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_label_call(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_char_lit(tasm_parser_t* parser);
tasm_ast_t* tasm_parse_num_lit(tasm_parser_t* parser);

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


tasm_ast_t* tasm_parse_line(tasm_parser_t* parser) {
    if (is_line_label_decl(parser))
        return tasm_parse_label_decl(parser);
    if (is_line_proc(parser))
        return tasm_parse_proc(parser);
    if (is_line_instruction(parser))
        return tasm_parse_instruction(parser);

    if (parser->current_token.type == TOKEN_ID)
        tasm_parser_eat(parser, 3200);

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
        tasm_parser_eat(parser, 6400);

    if (parser->current_token.type == TOKEN_ID)
        tasm_parser_eat(parser, 3200);

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
        if (operand == NULL) tasm_parser_eat(parser, 1400);
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
        if (operand == NULL) tasm_parser_eat(parser, 2000);
        break;
    case TOKEN_OP_SWAP: tag = AST_OP_SWAP;
        operand = tasm_parse_int_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, 2000);
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
        if (operand == NULL) tasm_parser_eat(parser, 1400);
        break;
    case TOKEN_OP_JZ: tag = AST_OP_JZ;
        operand = tasm_parse_jmp_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, 1400);
        break;
    case TOKEN_OP_JNZ: tag = AST_OP_JNZ;
        operand = tasm_parse_jmp_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, 1400);
        break;
    case TOKEN_OP_CALL: tag = AST_OP_CALL;
        operand = tasm_parse_label_operand(parser);
        if (operand == NULL) tasm_parser_eat(parser, 1400);
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

tasm_ast_t* tasm_parse_number_operand(tasm_parser_t *parser) {
    if (parser->current_token.type == TOKEN_DECIMAL_NUMBER ||
        parser->current_token.type == TOKEN_FLOAT_NUMBER ||
        parser->current_token.type == TOKEN_HEX_NUMBER ||
        parser->current_token.type == TOKEN_BINARY_NUMBER) {
        return tasm_parse_num_lit(parser);
    }
    return NULL;
}

tasm_ast_t* tasm_parse_int_operand(tasm_parser_t *parser) {
    if (is_operand_int(parser))
        return tasm_parse_num_lit(parser);
    return NULL;
}

tasm_ast_t* tasm_parse_jmp_operand(tasm_parser_t* parser) {
    if (is_operand_int(parser))
        return tasm_parse_num_lit(parser);
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
    if (is_operand_number(parser))
        return tasm_parse_num_lit(parser);
    if (is_operand_char(parser))
        return tasm_parse_char_lit(parser);

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

tasm_ast_t* tasm_parse_num_lit(tasm_parser_t* parser) {
    const char* text_val = parser->current_token.value;
    token_type_t type = parser->current_token.type;
    uint32_t value;
    switch (type)
    {
    case TOKEN_DECIMAL_NUMBER: {
        int val = atoi(text_val);
        value = *(uint32_t*)&val;
        break;
    }
    case TOKEN_FLOAT_NUMBER: {
        float val = atof(text_val);
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
        .number.value.u32 = value,
        // FIXME: find a way to support for floating or hex vals
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
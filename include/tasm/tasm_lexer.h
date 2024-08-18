#ifndef TASM_LEXER_H_
#define TASM_LEXER_H_

#include <stddef.h>
#include <stdbool.h>
#include <tasm/tasm_token.h>
#include <common/arena.h>

#define TOKENS_ARENA_CAPACITY 2048

typedef struct {
    int row, col;
    const char* file_name;
} loc_t;

typedef struct {
    size_t cursor;
    char prev_char;
    char current_char;
    char next_char;
    const char* source_code;
    size_t source_code_size;
    loc_t loc;
    arena_t tokens_arena;
} tasm_lexer_t;

tasm_lexer_t tasm_lexer_init(const char* src);
void tasm_lexer_destroy(tasm_lexer_t* lexer);

void tasm_lexer_advance(tasm_lexer_t* lexer);
void tasm_lexer_skip_whitespace(tasm_lexer_t* lexer);
void tasm_lexer_skip_line(tasm_lexer_t* lexer);

tasm_token_t tasm_lexer_get_next_token(tasm_lexer_t* lexer);

tasm_token_t lexer_collect_one_chars(tasm_lexer_t* lexer);

tasm_token_t tasm_lexer_collect_id(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_number(tasm_lexer_t *lexer);

bool is_id_op(token_type_t type, char* val);

#ifdef TASM_LEXER_IMPLEMENTATION

#include <ctype.h>

tasm_lexer_t tasm_lexer_init(const char* src) {
    tasm_lexer_t lexer = {
        .cursor = 0,
        .source_code = src,
        .prev_char = src[0],
        .current_char = src[0],
        .next_char = src[1],
        .source_code_size = strlen(src),
        .loc.row = 0,
        .loc.col = 0,
        .loc.file_name = NULL, // TODO: take file name
        .tokens_arena = arena_init(TOKENS_ARENA_CAPACITY),
    };
    return lexer;
}

void tasm_lexer_destroy(tasm_lexer_t *lexer) {
    arena_destroy(&lexer->tokens_arena);
}

void tasm_lexer_advance(tasm_lexer_t* lexer) {
    if (lexer->current_char == '\n') {
        lexer->loc.row++;
        lexer->loc.col = 0;
    }
    if (lexer->current_char != EOF
    && lexer->cursor < lexer->source_code_size) {
        lexer->prev_char = lexer->current_char;
        lexer->current_char = lexer->next_char;
        lexer->next_char = lexer->source_code[++lexer->cursor + 1];
        lexer->loc.col++;
    }
}

void tasm_lexer_skip_whitespace(tasm_lexer_t* lexer) {
    while (lexer->current_char == ' '
    || lexer->current_char == '\t') {
        tasm_lexer_advance(lexer);
    }
}

void tasm_lexer_skip_line(tasm_lexer_t* lexer) {
    while (lexer->current_char != '\n' && lexer->current_char != EOF) {
        tasm_lexer_advance(lexer);
    }
}

tasm_token_t tasm_lexer_get_next_token(tasm_lexer_t* lexer) {
    tasm_lexer_skip_whitespace(lexer);

    if (isalpha(lexer->current_char))
        return tasm_lexer_collect_id(lexer);
    if (isdigit(lexer->current_char))
        return tasm_lexer_collect_number(lexer);

    return lexer_collect_one_chars(lexer);
    // if (r.type == TOKEN_NONE) {
    //     fprintf(stderr, "file:%d:%d: Unknown token type!\n", lexer->loc.row, lexer->loc.col);
    //     // exit(-1);
    // }

    // return r;
}

tasm_token_t lexer_collect_one_chars(tasm_lexer_t *lexer) {
    switch (lexer->current_char)
    {
    case '\n':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_ENDLINE, "endl");
    
    case ';':
        tasm_lexer_skip_line(lexer);
        return tasm_token_create(TOKEN_COMMENT, ";");
    
    case ':':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_COLON, ":");

    case '\'':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_APOST, "'");

    case EOF:
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_EOF, "eof");

    default:
        return tasm_token_create(TOKEN_NONE, NULL);
    }
}

tasm_token_t tasm_lexer_collect_id(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    while (isalnum(lexer->current_char)) {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);
    }
    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(TOKEN_ID, val);

    for (token_type_t i = INSTRUCTIONS_TOKEN_BEGIN; i < INSTRUCTIONS_TOKEN_END - 1; i++) {
        if (is_id_op(i, val))
            return tasm_token_create(i + 1, val);
    }
    if (strcmp("proc", val) == 0)
        return tasm_token_create(TOKEN_PROC, val);
    if (strcmp("endp", val) == 0)
        return tasm_token_create(TOKEN_ENDP, val);

    return token;
}

tasm_token_t tasm_lexer_collect_number(tasm_lexer_t* lexer) {
    size_t len = 0;
    char temp_val[128];
    while ((isdigit(lexer->current_char))) {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);
    }
    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(TOKEN_NUMBER, val);
    return token;
}

const size_t _inst_strings_count = 30;

const char* _inst_strings_lower[] = {
    "nop", "push", 
    "add", "sub", "mult", "div", 
    "mod", 
    "dup", "cln", "swap",
    "addf", "subf", "multf", "divf",
    "inc", "incf", "dec", "decf",
    "jmp","jz" ,"jnz", "call", "ret",
    "ci2f", "ci2u", "cf2i", "cf2u", "cu2i", "cu2f",
    "hlt"
};

const char* _inst_strings_upper[] = {
    "NOP", "PUSH", 
    "ADD", "SUB", "MULT", "DIV", 
    "MOD", 
    "DUP", "CLN", "SWAP",
    "ADDF", "SUBF", "MULTF", "DIVF",
    "INC", "INCF", "DEC", "DECF",
    "JMP","JZ" ,"JNZ", "CALL", "RET",
    "CI2F", "CI2U", "CF2I", "CF2U", "CU2I", "CU2F",
    "HLT"
};

bool is_id_op(token_type_t type, char* val) {
    if (strcmp(val,
        _inst_strings_lower[type - INSTRUCTIONS_TOKEN_BEGIN]
    ) == 0 ||
    strcmp(val,
        _inst_strings_upper[type - INSTRUCTIONS_TOKEN_BEGIN]
    ) == 0) {
        return true;
    }
    return false;
}

#endif//TASM_TOKEN_IMPLEMENTATION

#endif//TASM_LEXER_H_

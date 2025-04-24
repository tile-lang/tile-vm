#ifndef TASM_LEXER_H_
#define TASM_LEXER_H_

#include <stddef.h>
#include <stdbool.h>
#include <tasm/tasm_token.h>
#include <common/arena.h>
#include <common/cmd_colors.h>

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
    arena_t* tokens_arena;
} tasm_lexer_t;

tasm_lexer_t tasm_lexer_init(const char* src, const char* file_name);
void tasm_lexer_destroy(tasm_lexer_t* lexer);

void tasm_lexer_advance(tasm_lexer_t* lexer);
void tasm_lexer_skip_whitespace(tasm_lexer_t* lexer);
void tasm_lexer_skip_line(tasm_lexer_t* lexer);
// Peek function checks the next character without advancing
// if you call 2 times it peeks 2 character ahead
char tasm_lexer_peek(tasm_lexer_t* lexer);
// Peek reset function resets the peek function
void tasm_lexer_peek_reset(tasm_lexer_t *lexer);

tasm_token_t tasm_lexer_get_next_token(tasm_lexer_t* lexer);

tasm_token_t lexer_collect_one_chars(tasm_lexer_t* lexer);

tasm_token_t tasm_lexer_collect_id(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_str(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_char(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_number(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_hex_number(tasm_lexer_t *lexer);
tasm_token_t tasm_lexer_collect_binary_number(tasm_lexer_t *lexer);

bool is_id_op(token_type_t type, char* val);
bool is_id_ctype(token_type_t type, char* val);
bool isbinprefix(char first, char second);
bool ishexprefix(char first, char second);

#ifdef TASM_LEXER_IMPLEMENTATION

#include <ctype.h>

tasm_lexer_t tasm_lexer_init(const char* src, const char* file_name) {
    tasm_lexer_t lexer = {
        .cursor = 0,
        .source_code = src,
        .prev_char = src[0],
        .current_char = src[0],
        .next_char = src[1],
        .source_code_size = strlen(src),
        .loc.row = 1,
        .loc.col = 0,
        .loc.file_name = file_name,
        .tokens_arena = arena_init(TOKENS_ARENA_CAPACITY),
    };
    return lexer;
}

void tasm_lexer_destroy(tasm_lexer_t *lexer) {
    arena_destroy(lexer->tokens_arena);
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
static char tasm_lexer_peek_upgraded(tasm_lexer_t* lexer, bool reset) {
    static size_t peek = 0;
    if(peek + lexer->cursor >= lexer->source_code_size)
        return '\0';
    peek++;
    if (reset)
        peek = 0;
    return lexer->source_code[lexer->cursor + peek - 1];
}

char tasm_lexer_peek(tasm_lexer_t *lexer) {
    return tasm_lexer_peek_upgraded(lexer, false);
}

void tasm_lexer_peek_reset(tasm_lexer_t *lexer) {
    tasm_lexer_peek_upgraded(lexer, true);
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
    if (lexer->prev_char == '"' && lexer->current_char != '"'&& lexer->current_char != '\n' && lexer->current_char != EOF && lexer->current_char != '\'')
        return tasm_lexer_collect_str(lexer);
    if (lexer->prev_char == '\'' && lexer->current_char != '\n' && lexer->current_char != EOF && lexer->current_char != '"')
        return tasm_lexer_collect_char(lexer);
    
    tasm_lexer_skip_whitespace(lexer);

    if (isalpha(lexer->current_char) || lexer->current_char == '_')
        return tasm_lexer_collect_id(lexer);
    if (isbinprefix(lexer->current_char, lexer->next_char))
        return tasm_lexer_collect_binary_number(lexer);
    if (ishexprefix(lexer->current_char, lexer->next_char))
        return tasm_lexer_collect_hex_number(lexer);
    if (isdigit(lexer->current_char))
        return tasm_lexer_collect_number(lexer);

    return lexer_collect_one_chars(lexer);

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

    case '"':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_QUOTA, "\"");
    
    case '@':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_AT, "@");
    
    case '-':
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_MINUS, "-");

    case EOF:
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_EOF, "eof");

    default:
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_NONE, NULL);
    }
}

tasm_token_t tasm_lexer_collect_id(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    while (isalnum(lexer->current_char) || lexer->current_char == '_') {
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

    // lex c interface function declerations
    for (token_type_t i = TOKEN_TCI_BEGIN; i < TOKEN_TCI_END - 1; i++) {
        if (is_id_ctype(i, val))
            return tasm_token_create(i + 1, val);
    }
    if (strcmp("cfun", val) == 0)
        return tasm_token_create(TOKEN_CFUNCTION, val);
    if (strcmp("data", val) == 0)
        return tasm_token_create(TOKEN_DATA, val);

    return token;
}

tasm_token_t tasm_lexer_collect_str(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    // tasm_lexer_advance(lexer);
    size_t line_end = 0;
    char c = tasm_lexer_peek(lexer);
    while (c != '\n' && c != EOF) {
        line_end++;
        c = tasm_lexer_peek(lexer);
    }
    tasm_lexer_peek_reset(lexer);
    while (lexer->current_char != '"') {
        if (len > line_end) {
            printf("%s:%d:%d: "CLR_RED"ERROR"CLR_END" missing string quota '\"'\n",
                lexer->loc.file_name,
                lexer->loc.row,
                lexer->loc.col
            );
            break;
        }
        // FIXME: '\0' doesn't work for some reason?!
        if (lexer->current_char == '\\') {
            tasm_lexer_advance(lexer); // advance to escape character
            switch (lexer->current_char) {
                case 'n':  temp_val[len++] = '\n'; break;
                case 't':  temp_val[len++] = '\t'; break;
                case 'r':  temp_val[len++] = '\r'; break;
                case '\\': temp_val[len++] = '\\'; break;
                case '"':  temp_val[len++] = '"';  break;
                case '0':  temp_val[len++] = '\0'; break;
                default:
                    printf("%s:%d:%d: "CLR_RED"ERROR"CLR_END" unknown escape sequence '\\%c'\n",
                        lexer->loc.file_name,
                        lexer->loc.row,
                        lexer->loc.col,
                        lexer->current_char);
                    break;
            }
        } else {
            temp_val[len++] = lexer->current_char;
        }

        tasm_lexer_advance(lexer);
    }

    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(TOKEN_STRING, val);
    return token;
}

tasm_token_t tasm_lexer_collect_char(tasm_lexer_t *lexer) {
    char temp_val[2];
    temp_val[0] = lexer->current_char;
    temp_val[1] = '\0';
    
    tasm_lexer_advance(lexer);
    if (lexer->current_char != '\'') {
        tasm_lexer_advance(lexer);
        return tasm_token_create(TOKEN_NONE, NULL);
    }

    char* val = (char*)arena_alloc(&lexer->tokens_arena, 2);
    memmove(val, temp_val, 2);
    tasm_token_t token = tasm_token_create(TOKEN_CHAR, val);
    return token;
}

tasm_token_t tasm_lexer_collect_number(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    token_type_t type = TOKEN_FLOAT_NUMBER;
    
    while (isdigit(lexer->current_char)) {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);
    }

    if (lexer->current_char == '.') {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);

        while (isdigit(lexer->current_char)) {
            temp_val[len] = lexer->current_char;
            len++;
            tasm_lexer_advance(lexer);
        }
    
    }
    else {
        type = TOKEN_DECIMAL_NUMBER;
    }

    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(type, val);
    return token;
}

tasm_token_t tasm_lexer_collect_hex_number(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    tasm_lexer_advance(lexer);
    tasm_lexer_advance(lexer);
    while ((isxdigit(lexer->current_char))) {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);
    }
    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(TOKEN_HEX_NUMBER, val);
    return token;
}

tasm_token_t tasm_lexer_collect_binary_number(tasm_lexer_t *lexer) {
    size_t len = 0;
    char temp_val[128];
    tasm_lexer_advance(lexer);
    tasm_lexer_advance(lexer);
    while (lexer->current_char == '0' || lexer->current_char == '1' ) {
        temp_val[len] = lexer->current_char;
        len++;
        tasm_lexer_advance(lexer);
    }
    temp_val[len] = '\0';
    len++;
    char* val = (char*)arena_alloc(&lexer->tokens_arena, len);
    memmove(val, temp_val, len);
    tasm_token_t token = tasm_token_create(TOKEN_BINARY_NUMBER, val);
    return token;
}

const size_t _inst_strings_count = 60;

const char* _inst_strings_lower[] = {
    "nop", "push", "pop",
    "add", "sub", "mult", "div", 
    "mod", 
    "dup", "cln", "swap",
    "addf", "subf", "multf", "divf",
    "inc", "incf", "dec", "decf",
    "jmp","jz" ,"jnz", "call", "ret",
    "ci2f", "ci2u", "cf2i", "cf2u", "cu2i", "cu2f",
    "gt", "gtf", "lt", "ltf", "eq", "eqf", "ge", "gef", "le", "lef",
    "and", "or", "not",
    "band", "bor", "bnot", "lshft", "rshft",
    "loadc", "aloadc", "load",  "store", "gload",  "gstore",
    "halloc", "deref", "hset",
    "puts",
    "native",
    "hlt"
};

const char* _inst_strings_upper[] = {
    "NOP", "PUSH", "POP",
    "ADD", "SUB", "MULT", "DIV", 
    "MOD", 
    "DUP", "CLN", "SWAP",
    "ADDF", "SUBF", "MULTF", "DIVF",
    "INC", "INCF", "DEC", "DECF",
    "JMP","JZ" ,"JNZ", "CALL", "RET",
    "CI2F", "CI2U", "CF2I", "CF2U", "CU2I", "CU2F",
    "GT", "GTF", "LT", "LTF", "EQ", "EQF", "GE", "GEF", "LE", "LEF",
    "AND", "OR", "NOT",
    "BAND", "BOR", "BNOT", "LSHFT", "RSHFT",
    "LOADC", "ALOADC", "LOAD",  "STORE", "GLOAD",  "GSTORE",
    "HALLOC", "DEREF", "HSET",
    "PUTS",
    "NATIVE",
    "HLT"
};

const size_t _tci_ctypes_count = 12;

const char* _tci_ctypes[] = {
    "u8", "u16", "u32", "u64",
    "i8", "i16", "i32", "i64",
    "f32", "f64",
    "ptr", "void",
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

bool is_id_ctype(token_type_t type, char* val) {
    if (strcmp(val, _tci_ctypes[type - TOKEN_TCI_BEGIN]) == 0) {
        return true;
    }
    return false;
}

bool isbinprefix(char first, char second) {
    if (first == '0' && (second == 'b' || second == 'B'))
        return true;
    return false;
}

bool ishexprefix(char first, char second) {
    if (first == '0' && (second == 'x' || second == 'X'))
        return true;
    return false;
}

#endif//TASM_TOKEN_IMPLEMENTATION

#endif//TASM_LEXER_H_

#ifndef TASM_TOKEN_H_
#define TASM_TOKEN_H_

typedef enum {
    TOKEN_NONE,
    TOKEN_ID,
    TOKEN_PROC,
    TOKEN_ENDP,
    TOKEN_COLON,
    TOKEN_APOST,

    INSTRUCTIONS_TOKEN_BEGIN,

    TOKEN_OP_NOP,
    TOKEN_OP_PUSH,
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_MULT,
    TOKEN_OP_DIV,
    TOKEN_OP_MOD,
    TOKEN_OP_DUP,
    TOKEN_OP_CLN,
    TOKEN_OP_SWAP,
    TOKEN_OP_ADDF,
    TOKEN_OP_SUBF,
    TOKEN_OP_MULTF,
    TOKEN_OP_DIVF,
    TOKEN_OP_INC,
    TOKEN_OP_INCF,
    TOKEN_OP_DEC,
    TOKEN_OP_DECF,
    TOKEN_OP_JMP,
    TOKEN_OP_JZ,
    TOKEN_OP_JNZ,
    TOKEN_OP_CALL,
    TOKEN_OP_RET,
    TOKEN_OP_CI2F,
    TOKEN_OP_CI2U,
    TOKEN_OP_CF2I,
    TOKEN_OP_CF2U,
    TOKEN_OP_CU2I,
    TOKEN_OP_CU2F,
    TOKEN_OP_HALT,

    INSTRUCTIONS_TOKEN_END,


    TOKEN_DECIMAL_NUMBER,
    TOKEN_FLOAT_NUMBER,
    TOKEN_HEX_NUMBER,
    TOKEN_BINARY_NUMBER,
    TOKEN_COMMENT,
    TOKEN_ENDLINE,
    TOKEN_EOF,
} token_type_t;

typedef struct {
    token_type_t type;
    char* value;
} tasm_token_t;

tasm_token_t tasm_token_create(token_type_t type, char* value);

#ifdef TASM_TOKEN_IMPLEMENTATION

tasm_token_t tasm_token_create(token_type_t type, char* value) {
    return (tasm_token_t) {
        .type = type,
        .value = value
    };
}

#endif//TASM_TOKEN_IMPLEMENTATION

#endif//TTOKEN_H_

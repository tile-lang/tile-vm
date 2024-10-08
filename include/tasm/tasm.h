#include <common/tdef.h>

TDEF_EXTERN_C_BEGIN

#define TASM_TOKEN_IMPLEMENTATION
#include <tasm/tasm_token.h>
#define ARENA_IMPLEMENTATION
#include <common/arena.h>
#define TASM_LEXER_IMPLEMENTATION
#include <tasm/tasm_lexer.h>
#define TASM_AST_IMPLEMENTATION
#include <tasm/tasm_ast.h>
#define TASM_PARSER_IMPLEMENTATION
#include <tasm/tasm_parser.h>
#define TASM_TRANSLATOR_IMPLEMENTATION
#include <tasm/tasm_translator.h>

TDEF_EXTERN_C_END

#include <tasm/tasm.h>

arena_t src_arena;

char* read_file_content(const char* file_name) {
    FILE* src_file = fopen(file_name, "r");

    if (src_file == NULL)
        printf("file can't be opened!\n");

    fseek(src_file, 0L, SEEK_END);
    unsigned int file_size = ftell(src_file);
    fseek(src_file, 0L, SEEK_SET);

    char* content = arena_alloc(&src_arena, file_size);

    char ch = 0;
    for (size_t i = 0; ch != EOF; i++) {
        ch = fgetc(src_file);
        content[i] = ch;
    }

    return content;
}

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);

    ast_arena = arena_init(1024);
    src_arena = arena_init(1024);

    char* content = read_file_content("factorial.tasm");

    tasm_lexer_t lexer = tasm_lexer_init(
        content
    );

    // tasm_token_t t = tasm_token_create(TOKEN_NONE, NULL);
    // while (t.type != TOKEN_EOF) {
    //     int r = lexer.loc.row;
    //     int c = lexer.loc.col;
    //     t = tasm_lexer_get_next_token(&lexer);
    //     printf("%d:%d: TOKEN(%s : %d)\n", r, c, t.value, t.type);
    // }
    
    tasm_parser_t parser = tasm_parser_init(&lexer);

    tasm_ast_t* ast = tasm_parse_file(&parser);
    tasm_ast_show(ast, 0);

    tasm_parser_destroy(&parser);

    arena_destroy(&src_arena);
    arena_destroy(&ast_arena);


    return 0;
}
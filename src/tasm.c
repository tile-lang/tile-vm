#include <tasm/tasm.h>
#include <common/cmd_colors.h>

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

bool tasm_usage(int argc) {
    if (argc < 2) {
        fprintf(stdout, CLR_RED"Invalid usage!"CLR_END "can not found input file.\n");
        fprintf(stdout, "    tasm <file.tasm>\n");
        return false;
    }
    if (argc >= 3) {
        fprintf(stdout, CLR_YELLOW"Warning: "CLR_END "more than expected input\n");
        fprintf(stdout, "    tasm <file.tasm>\n");
    }
    return true;
}

int main(int argc, char **argv) {
    
    if (!tasm_usage(argc)) {
        return -1;
    }

    char* file_name = argv[1];

    ast_arena = arena_init(1024);
    src_arena = arena_init(1024);

    char* content = read_file_content(file_name);

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

    tasm_translator_t translator = tasm_translator_init();
    
    tasm_resolve_labels(&translator, ast);
    // tasm_resolve_label_calls(&translator, ast);
    
    symbol_dump(&translator);

    tasm_translate_unit(&translator, ast);
    tasm_translator_generate_bin(&translator);
    
    tasm_translator_destroy(&translator);


    tasm_parser_destroy(&parser);

    arena_destroy(&src_arena);
    arena_destroy(&ast_arena);


    return 0;
}
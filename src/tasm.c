#include <tasm/tasm.h>
#include <common/cmd_colors.h>
#include <tasmc/tasmc.h>
#define CLI_IMPLEMENTATION
#include <common/cli.h>

arena_t* src_arena;

char* read_file_content(const char* file_name) {
    FILE* src_file = fopen(file_name, "r");

    if (src_file == NULL) {
        printf(CLR_RED"File can't be opened: "CLR_END"%s\n", file_name);
        fclose(src_file);
        exit(1);
    }

    fseek(src_file, 0L, SEEK_END);
    unsigned int file_size = ftell(src_file);
    fseek(src_file, 0L, SEEK_SET);

    if (file_size == 0) {
        printf(CLR_RED"File is empty: "CLR_END"%s\n", file_name);
        fclose(src_file);
        exit(1);
    }

    char* content = arena_alloc(&src_arena, file_size);

    char ch = 0;
    for (size_t i = 0; ch != EOF; i++) {
        ch = fgetc(src_file);
        content[i] = ch;
    }
    fclose(src_file);
    
    return content;
}


int main(int argc, char **argv) {
    
    cli_parsed_args_t args = {
        .compile = 0,
        .file_name = NULL,
        .output_name = NULL,
        .clib_count = 0,
        .ast_show = false,
    };

    if (!cli_tasm_parse_command_line(&args, &argc, &argv))
        return EXIT_FAILURE;


    ast_arena = arena_init(1024);
    // FIXME: do not use arena for file reading!!!
    src_arena = arena_init(4096*16);

    char* content = read_file_content(args.file_name);

    tasm_lexer_t lexer = tasm_lexer_init(
        content,
        args.file_name
    );

    // tasm_token_t t = tasm_token_create(TOKEN_NONE, NULL);
    // while (t.type != TOKEN_EOF) {
    //     int r = lexer.loc.row;
    //     int c = lexer.loc.col;
    //     t = tasm_lexer_get_next_token(&lexer);
    //     printf("%d:%d: TOKEN(%s : %d)\n", r, c, t.value, t.type);
    // }
    // exit(EXIT_FAILURE);
    
    tasm_parser_t parser = tasm_parser_init(&lexer);

    tasm_ast_t* ast = tasm_parse_file(&parser);

    if (tasm_parser_is_err(&parser)) {
        tasm_parser_destroy(&parser);
        tasm_ast_destroy(ast);
        arena_destroy(ast_arena);
        arena_destroy(src_arena);
        exit(EXIT_FAILURE);
    }

    if (args.ast_show) {
        tasm_ast_show(ast, 0);
    }


    tasm_translator_t translator = tasm_translator_init();
    
    tasm_resolve_procs(&translator, ast);
    tasm_resolve_labels(&translator, ast, NULL);
    // tasm_resolve_label_calls(&translator, ast);
    
    // symbol_dump(&translator);

    tasm_translate_unit(&translator, ast);
    if (!tasm_translator_is_err(&translator)) {
        tasm_translator_generate_bin(&translator, args);
        if (args.compile) {
            tasmc_init("out.asm");
            tasmc_compile_nasm(ast);
            tasmc_destroy();
        }
    }
    
    tasm_translator_destroy(&translator);


    tasm_parser_destroy(&parser);

    tasm_ast_destroy(ast);

    arena_destroy(ast_arena);
    arena_destroy(src_arena);


    return 0;
}
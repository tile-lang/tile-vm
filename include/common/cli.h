#ifndef CLI_H
#define CLI_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <common/cmd_colors.h>

#define MAX_LIB_COUNT 32

typedef struct {
    char* file_name;
    char* output_name;

    const char* clib_names[MAX_LIB_COUNT];
    size_t clib_count;

    bool ast_show;
    bool compile; // that will run tasmc (tasm to nasm)
} cli_parsed_args_t;

bool cli_tasm_parse_command_line(cli_parsed_args_t* args, int* argc, char*** argv);
bool cli_tvm_parse_command_line(cli_parsed_args_t* args, int* argc, char*** argv);

#ifdef CLI_IMPLEMENTATION

static char __current_cli_option[32] = "no_option";

char* cli_shift(int *argc, char ***argv) {
    if (*argc <= 0)
        fprintf(stderr, "%s: %s\n"CLR_RED"Error"CLR_END, strcat(__current_cli_option, " expected a name!"));
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

bool cli_tasm_usage(int argc) {
    if (argc < 2) {
        fprintf(stdout, CLR_RED"Invalid usage!"CLR_END "can not found input file.\n");
        fprintf(stdout, "    tasm <file.tasm>\n");
        return false;
    }
    else if (argc >= 7) {
        fprintf(stdout, CLR_YELLOW"Warning: "CLR_END "more than expected input\n");
        fprintf(stdout, "    tasm <file.tasm>\n");
    }
    return true;
}

bool cli_tvm_usage(int argc) {
    if (argc < 2) {
        fprintf(stdout, CLR_RED"Invalid usage!"CLR_END" can not found input file.\n");
        fprintf(stdout, "    tvm <input.bin>\n");
        return false;
    }
    else {
        fprintf(stdout, CLR_YELLOW"Warning: "CLR_END" more than expected input\n");
        fprintf(stdout, "    tvm <input.bin>\n");
    }
    return true;
}

#define compare(x, y) strcmp(x, y) == 0 && (strcpy(__current_cli_option, (y)) != NULL)

bool cli_tasm_parse_command_line(cli_parsed_args_t* args, int* argc, char*** argv) {
    if (!cli_tasm_usage(*argc))
        return false;

    // neccessary flags
    bool _o = false;

    cli_shift(argc, argv); // ./tasm
    while (*argc > 0) {
        char* arg = cli_shift(argc, argv);
        if (compare(arg, "-o")) {
            args->output_name = cli_shift(argc, argv);
            _o = true;
        }
        else if (compare(arg, "-c"))
            args->compile = true;
        else if (compare(arg, "-l"))
            args->clib_names[args->clib_count++] = cli_shift(argc, argv);
        else if (compare(arg, "-ast"))
            args->ast_show = true;
        else
            args->file_name = arg;
    }
    
    if (!args->file_name)
        return false;

    if (!_o)
        args->output_name = "out.bin";

    return true;
}

bool cli_tvm_parse_command_line(cli_parsed_args_t* args, int* argc, char*** argv) {
    if (!cli_tvm_usage(*argc))
        return false;

    cli_shift(argc, argv); // ./tvm
    while (*argc > 0) {
        char* arg = cli_shift(argc, argv);
        args->file_name = arg;
    }
    
    if (!(args->file_name))
        return false;
    return true;
}

#endif//CLI_IMPLEMENTATION

#endif//CLI_H

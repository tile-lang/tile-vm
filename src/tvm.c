#ifdef _WIN32
#include <windows.h>
// #include <locale.h>
#endif

#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>
#include <tvm/tci.h>

#define CLI_IMPLEMENTATION
#include <common/cli.h>

extern tci_t tci_instance;

int main(int argc, char **argv) {
    
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    cli_parsed_args_t args = {
        .compile = 0,
        .file_name = NULL,
        .output_name = NULL,
        .clib_count = 0,
    };
    
    if (!cli_tvm_parse_command_line(&args, &argc, &argv))
        return EXIT_FAILURE;


    tci_instance = tci_init();
    tvm_t vm = tvm_init();

    tvm_load_program_from_file(&vm, args.file_name);

    if (vm.program.metadata.module_count > 0) {
        //FIXME: support for multiple modules
        const char* module_name = vm.program.metadata.modules[0].module_name;
        printf("xx:%s\n", module_name);
        tci_load_module(&tci_instance, module_name);
        tci_metaprogram_to_ffi(&tci_instance, &vm);
    }

    tvm_run(&vm);

    tci_unload_all(&tci_instance);
    tvm_destroy(&vm);

    tci_destroy(&tci_instance);

    return 0;
}

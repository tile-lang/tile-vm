#define TVM_IMPLEMENTATION
#include <tvm/tvm.h>
#include <tvm/tci.h>

#define MAX_LIB_COUNT 1

typedef struct {
    const char* lib_names[MAX_LIB_COUNT];
    size_t lib_count;
    const char* bin_file_name;
} tvm_usage_t;
tvm_usage_t g_usage;

bool tvm_usage(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stdout, CLR_RED"Invalid usage!"CLR_END" can not found input file.\n");
        fprintf(stdout, "    tvm <out.bin>\n");
        return false;
    } else if (argc == 3) {
        g_usage.lib_names[0] = argv[2];
        g_usage.lib_count = 1;
        // TODO: implement a parsing system for command line arguments
    }
    else if (argc >= 4) {
        fprintf(stdout, CLR_YELLOW"Warning: "CLR_END" more than expected input\n");
        fprintf(stdout, "    tvm <out.bin>\n");
    }
    g_usage.bin_file_name = argv[1];
    return true;
}

extern tci_t tci_instance;

int main(int argc, char **argv) {
    if (!tvm_usage(argc, argv))
        return -1;

    tci_instance = tci_init();
    tvm_t vm = tvm_init();

    tvm_load_program_from_file(&vm, g_usage.bin_file_name);

    if (g_usage.lib_count > 0) {
        const char* module_name = g_usage.lib_names[0];
        tci_load_module(&tci_instance, module_name);
        tci_metaprogram_to_ffi(&tci_instance, &vm);
    }

    tvm_run(&vm);

    tci_unload_all(&tci_instance);
    tvm_destroy(&vm);

    tci_destroy(&tci_instance);

    return 0;
}

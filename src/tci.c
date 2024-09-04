#include <stdio.h>
#include <common/cmd_colors.h>
#include <tvm/tci.h>

tci_t tci_instance;

tci_t tci_init() {
    return (tci_t) {
        .modules = {0},
        .module_count = 0,
        .ffi_arena = arena_init(4096),
    };
}

void tci_destroy(tci_t* instance) {
    arena_destroy(instance->ffi_arena);
}

void tci_load_module(tci_t *instance, const char *module_name)
{
    tci_module_handle_t lib;
#ifdef _WIN32
    lib = LoadLibrary(module_name);
    err_str_t last_err = GetLastError();
#else
    lib = dlopen(module_name, RTLD_LAZY);
    err_str_t last_err = dlerror();
#endif
    if (!lib) {
        fprintf(stderr, CLR_RED"Rutime library loading error at "CLR_END"%s: %s\n", module_name, last_err);
        exit(1);
    }
    instance->modules[instance->module_count].handle = lib;
    instance->modules[instance->module_count].name = module_name;
    instance->module_count++;
    fprintf(stdout, CLR_WHITE"Rutime library loaded "CLR_GREEN"successfully. "CLR_END"%s\n", module_name);
}

cfunptr_t tci_get_cfunction(tci_t* instance, /* TODO: give module name as param */ const char* func_name) {
#ifdef _WIN32
    cfunptr_t func_ptr = (cfunptr_t)GetProcAddress(instance->modules[instance->module_count - 1].handle, func_name);
#else
    cfunptr_t func_ptr = (cfunptr_t)dlsym(instance->modules[instance->module_count - 1].handle, func_name);
#endif
    if (!func_ptr) {
        fprintf(stderr, CLR_RED"tci error: "CLR_WHITE"could not find the "CLR_PINK"%s "CLR_END"function!\n", func_name);
        tci_unload_all(instance);
        return NULL;
    }
    return func_ptr;
}

ffi_type* tci_ctype_to_ffi_type(uint8_t ctype) {
    ffi_type* ftype;
    switch (ctype)
    {
    case CTYPE_UINT8: ftype = &ffi_type_uchar; break;
    case CTYPE_UINT16: ftype = &ffi_type_uint16; break;
    case CTYPE_UINT32: ftype = &ffi_type_uint32; break;
    case CTYPE_UINT64: ftype = &ffi_type_uint64; break;
    case CTYPE_INT8: ftype = &ffi_type_schar; break;
    case CTYPE_INT16: ftype = &ffi_type_sint16; break;
    case CTYPE_INT32: ftype = &ffi_type_sint32; break;
    case CTYPE_INT64: ftype = &ffi_type_sint64; break;
    case CTYPE_FLOAT32: ftype = &ffi_type_float; break;
    case CTYPE_FLOAT64: ftype = &ffi_type_double; break;
    case CTYPE_PTR: ftype = &ffi_type_pointer; break;
    case CTYPE_VOID: ftype = &ffi_type_void; break;
    default:
        break;
    }
    return ftype;
}

void tci_prepare_last_module(tci_t* instance, uint32_t native_func_count) {
    instance->modules[instance->module_count - 1].native_funcs = arena_alloc(instance->ffi_arena, sizeof(tci_native_func_t) * native_func_count);
    memset(instance->modules[instance->module_count - 1].native_funcs, 0, sizeof(tci_native_func_t) * native_func_count);
    instance->modules[instance->module_count - 1].native_func_count = native_func_count;
}

void tci_prepare_function(arena_t* arena, tci_native_func_t* function, uint8_t rtype, uint8_t* atypes, uint16_t acount) {
    ffi_type** args = arena_alloc(arena, sizeof(ffi_type*) * acount);
    ffi_type* ret = tci_ctype_to_ffi_type(rtype);
    for (size_t i = 0; i < acount; i++) {
        uint8_t atype = atypes[i];
        args[i] = tci_ctype_to_ffi_type(atype);
    }

    function->is_ok = ffi_prep_cif(&function->cif, FFI_DEFAULT_ABI, acount, ret, args) == FFI_OK;
}

void tci_metaprogram_to_ffi(tci_t* instance, tvm_t* vm) {
    uint32_t cfun_count = vm->program.metadata.cfun_count;
    tci_prepare_last_module(instance, cfun_count);

    for (size_t i = 0; i < cfun_count; i++) {
        uint16_t acount = vm->program.metadata.cfuns[i].acount;
        uint8_t rtype = vm->program.metadata.cfuns[i].rtype;
        uint8_t* atypes = vm->program.metadata.cfuns[i].atypes;

        tci_prepare_function(instance->ffi_arena, &instance->modules[instance->module_count - 1].native_funcs[i], rtype, atypes, acount);
    }
    
}

void tci_native_call(tvm_t* vm, uint32_t id, void *rvalue, void **avalues) {
    printf("id: %d\n", id);
    const char* proc_name = vm->program.metadata.cfuns[0].symbol_name;
    printf(CLR_BLUE"%s\n"CLR_END, proc_name);
    if (tci_instance.modules[tci_instance.module_count - 1].native_funcs[id].is_ok == true) {
        cfunptr_t func_ptr = tci_get_cfunction(&tci_instance, proc_name);
        ffi_call(&tci_instance.modules[tci_instance.module_count - 1].native_funcs[id].cif, FFI_FN(func_ptr), rvalue, avalues);
    }
}

void tci_unload_all(tci_t* instance) {
    for (size_t i = 0; i < instance->module_count; ++i) {
#ifdef _WIN32
        if (FreeLibrary(instance->modules[i].handle) == 0) {
            fprintf(stderr, CLR_RED"ERROR: could not unload module"CLR_END "%zu: %u\n", i, GetLastError());
            exit(1);
        }
#else
        if (dlclose(instance->modules[i].handle) != 0) {
            fprintf(stderr, CLR_RED"ERROR: could not unload module"CLR_END "%zu: %s\n", i, dlerror());
            exit(1);
        }
#endif
    }
}
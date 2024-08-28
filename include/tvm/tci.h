#ifndef TCI_H_
#define TCI_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <stdio.h>
#include <common/cmd_colors.h>

#include <ffi.h>

#ifdef _WIN32
    typedef HMODULE tci_module_t;
    typedef DWORD err_str_t;
#else
    typedef void* tci_module_t;
    typedef const char* err_str_t;
#endif

#define TCI_MODULE_CAPACITY 32

typedef struct {
    tci_module_t modules[TCI_MODULE_CAPACITY];
    const char*  module_names[TCI_MODULE_CAPACITY];
    size_t modules_size;
} tci_t;


void tci_load_library(tci_t* instance, const char* lib_name);
int test_ffi_puts(tci_t* instance);
void tci_unload_library(tci_t* instance, const char* lib_name); //TODO: implement this
void tci_unload_all(tci_t* instance);

#define TCI_IMPLEMENTATION
#ifdef TCI_IMPLEMENTATION

void tci_load_library(tci_t* instance, const char* lib_name) {
    tci_module_t lib;
#ifdef _WIN32
    lib = LoadLibrary(lib_name);
    err_str_t last_err = GetLastError();
#else
    lib = dlopen(lib_name, RTLD_LAZY);
    err_str_t last_err = dlerror();
#endif
    if (!lib) {
        fprintf(stderr, CLR_RED"Rutime library loading error at "CLR_END"%s: %s\n", lib_name, last_err);
        exit(1);
    }
    instance->modules[instance->modules_size] = lib;
    instance->module_names[instance->modules_size] = lib_name;
    instance->modules_size++;
    fprintf(stdout, CLR_WHITE"Rutime library loaded "CLR_GREEN"successfully. "CLR_END"%s\n", lib_name);
}

int test_ffi_puts(tci_t* instance) {

    FARPROC funcPointer = GetProcAddress(instance->modules[instance->modules_size - 1], "Add");
    if (!funcPointer) {
        printf("Could not find the Add function!\n");
        tci_unload_all(instance);
        return 1;
    }

    ffi_cif cif;
    ffi_type *args[2];
    void *values[2];
    ffi_arg rc;
    /* Initialize the argument info vectors */
    args[0] = &ffi_type_sint32;
    args[1] = &ffi_type_sint32;
    int a = 5;
    int b = 3;
    values[0] = &a;
    values[1] = &b;
    /* Initialize the cif */
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &ffi_type_sint32, args) == FFI_OK)
    {
        ffi_call(&cif, FFI_FN(funcPointer), &rc, values);
        printf("Result of Add(%d, %d): %d\n", a, b, (int)rc);
        // printf("Result of Add(%d, %d): ", a, b);
    } else {
        printf("ffi_prep_cif failed\n");
    }

    // printf("result: %d\n", result);

    return 0;
}

void tci_unload_library(tci_t* instance, const char* lib_name) {
    UNUSED_VAR(instance);
    UNUSED_VAR(lib_name);
}

void tci_unload_all(tci_t* instance) {
    for (size_t i = 0; i < instance->modules_size; ++i) {
#ifdef _WIN32
        if (FreeLibrary(instance->modules[i]) == 0) {
            fprintf(stderr, CLR_RED"ERROR: could not unload module"CLR_END "%zu: %u\n", i, GetLastError());
            exit(1);
        }
#else
        if (dlclose(instance->modules[i]) != 0) {
            fprintf(stderr, CLR_RED"ERROR: could not unload module"CLR_END "%zu: %s\n", i, dlerror());
            exit(1);
        }
#endif
    }
}


#endif//TCI_IMPLEMENTATION

#endif//TCI_H_
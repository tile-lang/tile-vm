#ifndef TCI_H_
#define TCI_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <common/arena.h>

#include <tvm/tvm.h>

#include <ffi.h>

#ifdef _WIN32
    typedef HMODULE tci_module_handle_t;
    typedef DWORD err_str_t;
    typedef FARPROC cfunptr_t;
#else
    typedef void* tci_module_handle_t;
    typedef const char* err_str_t;
    typedef void (*cfunptr_t)(void);
#endif

#define TCI_MODULE_CAPACITY 32

typedef struct {
    ffi_cif cif;
    bool is_ok;
} tci_native_func_t;

typedef struct {
    tci_module_handle_t handle;
    const char* name;
    tci_native_func_t* native_funcs;
    uint32_t native_func_count;
} tci_module_t;

typedef struct {
    tci_module_t modules[TCI_MODULE_CAPACITY];
    size_t module_count;
    arena_t* ffi_arena;
} tci_t;

tci_t tci_init();
void tci_destroy(tci_t* instance);

void tci_load_module(tci_t* instance, const char* module_name);
void tci_unload_all(tci_t* instance);


void tci_prepare_last_module(tci_t* instance, uint32_t native_func_count);
void tci_prepare_function(arena_t* arena, tci_native_func_t* function, uint8_t rtype, uint8_t* atypes, uint16_t acount);

cfunptr_t tci_get_cfunction(tci_t* instance, /* TODO: give module name as param */ const char* func_name);

void tci_metaprogram_to_ffi(tci_t* instance, tvm_t* vm);
void tci_native_call(tvm_t* vm, uint32_t id, void* rvalue, void** avalues);

ffi_type* tci_ctype_to_ffi_type(uint8_t ctype);

#endif//TCI_H_
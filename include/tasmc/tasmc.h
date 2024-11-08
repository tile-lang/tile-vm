#ifndef TASMC_H_
#define TASMC_H_

#include <tasm/tasm_ast.h>

void tasmc_init(const char* output);
void tasmc_destroy();
void tasmc_compile_nasm(tasm_ast_t* root);

#endif//TASMC_H_
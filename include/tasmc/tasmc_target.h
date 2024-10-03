#ifndef TASMC_TARGET_H_
#define TASMC_TARGET_H_

#define _T "\t"

#define TASMC_TARGET_x86_64_WIN32 \
"STD_OUTPUT_HANDLE equ -11\n" \
"\n" \
"extern GetStdHandle\n" \
"extern WriteFile\n" \
"extern ExitProcess\n" \
"\n" \

#define TASMC_TARGET_STACK \
"\n" \
"section .data\n" \
"stack_top:\n" \
"stack db 1024 dup(0)\n" \

// TODO: get stack capacity from tvm

#endif//TASMC_TARGET_H_
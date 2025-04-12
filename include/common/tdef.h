#ifndef TDEF_H_
#define TDEF_H_

#ifdef TDEF_EXTERN_C_BEGIN
    #undef TDEF_EXTERN_C_BEGIN
#endif
#ifdef TDEF_EXTERN_C_END
    #undef TDEF_EXTERN_C_END
#endif
#ifdef __cplusplus
    #define TDEF_EXTERN_C_BEGIN extern "C" {
    #define TDEF_EXTERN_C_END }
#else
    #define TDEF_EXTERN_C_BEGIN
    #define TDEF_EXTERN_C_END
#endif

TDEF_EXTERN_C_BEGIN

#ifdef _MSC_VER
    #if !defined(TDEF_WIN_SECURE_WARNINGS) && !defined(_CRT_SECURE_NO_WARNINGS)
        #define _CRT_SECURE_NO_WARNINGS
    #endif
#endif

#include <stdint.h>

uint8_t _is_little_endian() {
    volatile uint32_t i=0x01234567;
    // return 0 for big endian, 1 for little endian.
    return (*((uint8_t*)(&i))) == 0x67;
}

uint32_t swap_endian_uint32(uint32_t val) {
#ifdef __has_builtin
    #if __has_builtin(__builtin_bswap32)
        return __builtin_bswap32(val);
    #else
        return ((val >> 24) & 0x000000FF) |
                ((val >> 8)  & 0x0000FF00) |
                ((val << 8)  & 0x00FF0000) |
                ((val << 24) & 0xFF000000);
    #endif
#else
    return ((val >> 24) & 0x000000FF) |
            ((val >> 8)  & 0x0000FF00) |
            ((val << 8)  & 0x00FF0000) |
            ((val << 24) & 0xFF000000);
#endif
}

int32_t swap_endian_int32(int32_t val) {
    return (int32_t)swap_endian_uint32((uint32_t)val);
}

TDEF_EXTERN_C_END

#endif//TDEF_H_
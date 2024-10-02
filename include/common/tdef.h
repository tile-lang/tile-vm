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

TDEF_EXTERN_C_END

#endif//TDEF_H_
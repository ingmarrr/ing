/* ing.h - version 0.0.1

    DESCRIPTION:

    HISTORY:

*/

#ifndef ING
#define ING

#include <signal.h>
#include <sys/signal.h>
#ifndef ING_DEBUG
    #define ING_DEBUG
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
    #define ING_IS_WINDWOS
#elif defined(__APPLE__) || defined(__MACH__)
    #define ING_IS_MACOS
#elif defined(__linux__) || defined(__gnu_linux__)
    #define ING_IS_LINUX
#elif defined(__unix__) || defined(__unix)
    #define ING_IS_UNIX
#endif

#if defined(_MSC_VER)
    #define ING_COMPILER_MSVC
#elif defined(__clang__)
    #define ING_COMPILER_CLANG
#elif defined(__GNUC__)
    #define ING_COMPILER_GCC
#elif defined(__INTEL_COMPILER)
    #define ING_COMPILER_INTEL
#endif

#define ing_define static inline

#if defined(ING_COMPILER_MSVC)
    #define ing_mustuse _Check_return_
    #define ing_typeof(__type)  __typeof((__type))
    #define ing_alignof(__type) _alignof((__type))
    #define ing_forceinline __forceinline
    #define ing_restrict __restrict
#elif defined(ING_COMPILER_CLANG) || defined(ING_COMPILER_GCC)
    #define ing_mustuse __attribute__((warn_unused_result))
    #define ing_typeof(__type)  __typeof__((__type))
    #define ing_alignof(__type) __alignof__(__type)
    #define ing_forceinline __attribute__((always_inline)) inline
    #define ing_restrict __restrict__

    #define ing_format(__printf, __fmt, __args) __attribute__((format(__printf, __fmt, __args)))
#else
    #define ing_mustuse
    #define ing_typeof(__type)  typeof((__type))
    #define ing_alignof(__type) _Alignof((__type))
    #define ing_forceinline inline
    #define ing_restrict
#endif

#if defined(ING_COMPILER_MSVC)
    #define ing_likely(x) (x)
    #define ing_unlikely(x) (x)
#else
    #define ing_likely(x) __builtin_expect(!!(x), 1)
    #define ing_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#if defined(ING_COMPILER_MSVC)
    #define ing_prefetch(__addr) _mm_prefetch((const char*)(__addr), _MM_HINT_T0)
#else
    #define ing_prefetch(__addr) __builtin_prefetch(__addr)
#endif

#if defined(ING_COMPILER_MSVC)
    #define ing_function __FUNCTION__
#else
    #define ing_function __func__
#endif

#if defined(ING_COMPILER_MSVC)
    #define ing_assume(__expr) __assume(__expr)
#else
    #define ing_assume(__expr) do { if (!(__expr)) __builtin_unreachable(); } while (0)
#endif

#define ING_TODO(msg)           __ing_panic_handler("TODO", msg)
#define ING_NOT_IMPLEMENTED(msg)__ing_panic_handler("NOT IMPLEMENTED", msg)
#define ING_NOT_SUPPORTED(msg)  __ing_panic_handler("NOT SUPPORTED", msg)
#define ING_UNREACHABLE(msg)    __ing_panic_handler("UNREACHABLE", msg)
#define ING_PANIC(msg)          __ing_panic_handler("PANIC", msg)
#define ING_UNUSED(var)         (void)(var)

#define ING_UNEXPECTED(cond)    __builtin_expect(cond, 0)
#define ING_EXPECTED(cond)      __builtin_expect(cond, 1)

#define ING_ASSERT(cond, fmt, ...)                                                  \
    do {                                                                            \
        if (!(cond)) __ing_panic_handler("ASSERTION FAILED", fmt, ##__VA_ARGS__);   \
    } while(0)                                                                      \

#define ING_ASSERT_TYPE(var, type)                                                  \
    static_assert(_Generic((var),                                                   \
        type: 1,                                                                    \
        default: 0),                                                                \
    "Type mismatch")

#define ING_ASSERT_PTR_TYPE(ptr, type)                                              \
    static_assert(__builtin_types_compatible_p(typeof(ptr), type*),                 \
    "Pointer type mismatch")

#define ING_STATIC_ASSERT_SIZE(type, size)                                          \
    static_assert(sizeof(type) == size, "Size of " #type " must be " #size " bytes")\


#ifdef ING_DEBUG
    #define ING_DEBUG_ASSERT(cond, msg) ING_ASSERT(cond, msg)
#else
    #define ING_DEBUG_ASSERT(cond, msg) ((void)0)
#endif

#define __ing_panic_handler(title, fmt, ...)                                    \
    do {                                                                        \
        fprintf(stderr, "[%s:%d] %s: ", __FILE__, __LINE__, (title));           \
        fprintf(stderr, fmt, ##__VA_ARGS__);                                    \
        fprintf(stderr, "\n");                                                  \
        abort();                                                                \
    } while(0)                                                                  \

#define cast(type) (type)

typedef enum {
    Ing_Cmp_Less_Than,
    Ing_Cmp_Greater_Than,
    Ing_Cmp_Equals,
} Ing_Cmp_Result;

typedef enum {
    Ing_Err,
    Ing_Ok,
} Ing_Result_Kind;
ING_STATIC_ASSERT_SIZE(Ing_Result_Kind, 4);

typedef enum { Ing_None } Ing_None_Kind;

#define Ing_Result(Ok_Type, Err_Type)           \
    struct {                                    \
        Ing_Result_Kind kind;                   \
        union {                                 \
            Ok_Type     ok;                     \
            Err_Type    err;                    \
        } val;                                  \
    }

#define Ing_Maybe(Val_Type)                     \
    struct {                                    \
        Val_Type    value;                      \
        uint8_t     err;                        \
    }                                           \

#define ing_ok(out)  \
    { .kind = Ing_Ok, .val = { .ok = out } }

#define ing_err(out) \
    { .kind = Ing_Err, .val  = { .err = out } }


typedef char*       Ing_String;
typedef void*       Ing_Any_Ptr;

typedef uint8_t     Ing_U8;
typedef uint16_t    Ing_U16;
typedef uint32_t    Ing_U32;
typedef uint64_t    Ing_U64;
typedef uintptr_t   Ing_Usize;

typedef int8_t      Ing_I8;
typedef int16_t     Ing_I16;
typedef int32_t     Ing_I32;
typedef int64_t     Ing_I64;
typedef intptr_t    Ing_Isize;

typedef float       Ing_F32;
typedef double      Ing_F64;

typedef uint8_t     Ing_B8;
typedef uint16_t    Ing_B16;
typedef uint32_t    Ing_B32;
typedef uint64_t    Ing_B64;

#if defined (ING_IS_WINDOWS)
    #define ing_setsignal signal
#else
    ing_define void ing_setsignal(Ing_Isize sig, void (*handler)(int))
    {
        struct sigaction sa;
        sa.sa_handler   = handler;
        sa.sa_flags     = 0;
        sigemptyset(&sa.sa_mask);
        sigaction(sig, &sa, NULL);
    }
#endif

/**************************
-- BEGIN: NULLABILITY --
**************************/

#define ing_return_some(val)                                                    \
    do {                                                                        \
        ING_DEBUG_ASSERT(                                                       \
            val != NULL,                                                        \
            "[error] attempted to return null, when non-null was expected."     \
        );                                                                      \
        return val;                                                             \
    } while(0)                                                                  \

#define ing_return_defer(val)                                                   \
    do {                                                                        \
        out = (val);                                                            \
        goto defer;                                                             \
    } while(0)                                                                  \

#define ING_STRIP_PREFIX
#ifndef ING_PREFIX_GUARD_
#define ING_PREFIX_GUARD_
    #ifdef ING_STRIP_PREFIX

        typedef uint8_t  u8;
        typedef uint16_t u16;
        typedef uint32_t u32;
        typedef uint64_t u64;
        typedef size_t   usize;

        typedef int8_t   i8;
        typedef int16_t  i16;
        typedef int32_t  i32;
        typedef int64_t  i64;
        typedef intptr_t isize;

        typedef float    f32;
        typedef double   f64;

        typedef uint8_t  bool8;
        typedef uint16_t bool16;
        typedef uint32_t bool32;
        typedef uint64_t bool64;

        #define true     1
        #define false    0

    #endif
#endif

#ifdef ING_IMPL

#endif
#endif

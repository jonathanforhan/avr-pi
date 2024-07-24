#pragma once

#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef enum result {
    OK    = 0,
    ERROR = -1,
} result;

#define KB 1024U
#define MB 1024U * KB

#define GET_BIT(X, N) (((X) >> (N)) & 1)
#define SET_BIT(X, N) (X |= (1 << (N)))
#define CLR_BIT(X, N) (X &= ~(1 << (N)))

#define MAX(A, B)               \
    ({                          \
        __typeof__(A) _a = (A); \
        __typeof__(B) _b = (B); \
        _a > _b ? _a : _b;      \
    })

#define MIN(A, B)               \
    ({                          \
        __typeof__(A) _a = (A); \
        __typeof__(B) _b = (B); \
        _a < _b ? _a : _b;      \
    })

#define LOG_ERROR(MSG, ...) fprintf(stderr, "%s:%d ERROR " MSG "\n", __func__, __LINE__, ##__VA_ARGS__)

#ifdef NDEBUG
#define LOG_DEBUG(MSG, ...) (void)0
#else
#define LOG_DEBUG(MSG, ...) fprintf(stderr, "%s:%d DEBUG " MSG "\n", __func__, __LINE__, ##__VA_ARGS__)
#endif

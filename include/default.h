#ifndef __DEFAULT_H
#define __DEFAULT_H


/** 句柄定义 */
#define declear_handler(name)   struct name##__ {int unused;}; typedef struct name##__ * name

#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
#define C_API   extern "C"
#else
#define C_API
#endif // __cplusplus

#ifdef __cplusplus
#define C_API_BEGIN     extern "C" {
#define C_API_END       }
#else
#define C_API_BEGIN
#define C_API_END
#endif // __cplusplus

#if defined(__GNUC__)
    #include <stdint.h>

    #define section_(x)     __attribute__((section(x)))
    #define unused_(x)      __attribute__((unused))
    #define used_           __attribute__((used))
    #define align_(n)       __attribute__((aligned(n)))
    #define weak_           __attribute__((weak))
    #define inline_         static inline

#endif // defined



#endif

#include <owf.h>

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef OWF_PLATFORM_H
#define OWF_PLATFORM_H

/**
 * Basic platform detection
 */
#define OWF_PLATFORM_LINUX 0
#define OWF_PLATFORM_BSD 1
#define OWF_PLATFORM_DARWIN 2
#define OWF_PLATFORM_CYGWIN 3
#define OWF_PLATFORM_MINGW 4
#define OWF_PLATFORM_WINDOWS 5

#if defined(__GNUC__) && defined(__linux__)
    #define OWF_PLATFORM OWF_PLATFORM_LINUX
#elif defined(__GNUC__) && defined(__FreeBSD__)
    #define OWF_PLATFORM OWF_PLATFORM_BSD
#elif defined(__GNUC__) && defined(__APPLE__)
    #define OWF_PLATFORM OWF_PLATFORM_DARWIN
#elif defined(__GNUC__) && defined(__CYGWIN__)
    #define OWF_PLATFORM OWF_PLATFORM_CYGWIN
#elif defined(__GNUC__) && defined(__MINGW32__)
    #define OWF_PLATFORM OWF_PLATFORM_MINGW
#elif !defined(__GNUC__) && (defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64))
    #define OWF_PLATFORM OWF_PLATFORM_WINDOWS
#else
    #error "Don't know how to build on your platform!"
#endif

#if OWF_PLATFORM == OWF_PLATFORM_LINUX || OWF_PLATFORM == OWF_PLATFORM_BSD || OWF_PLATFORM ==  OWF_PLATFORM_DARWIN || OWF_PLATFORM == OWF_PLATFORM_CYGWIN || OWF_PLATFORM == OWF_PLATFORM_MINGW
    #define OWF_PLATFORM_IS_GNU 1
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #define OWF_PLATFORM_IS_GNU 0
#else
    #error "Don't know whether your platform is GNU or not!"
#endif

/**
 * Limits
 */
#if CHAR_MAX == INT8_MAX || CHAR_MAX == UINT8_MAX
    #define OWF_INT8_BITS 8
    #define OWF_CHAR_BITS 8
    #define OWF_CHAR_BYTES 1
#else
    #error "invalid CHAR_MAX value"
#endif

#if SHRT_MAX == INT16_MAX || SHRT_MAX == UINT16_MAX
    #define OWF_INT16_BITS 16
    #define OWF_SHORT_BITS 16
    #define OWF_SHORT_BYTES 2
#else
    #error "invalid SHRT_MAX value"
#endif

#if INT_MAX == INT32_MAX || INT_MAX == UINT32_MAX
    #define OWF_INT32_BITS 32
    #define OWF_INT_BITS 32
    #define OWF_INT_BYTES 4
#else
    #error "invalid INT_MAX value"
#endif

#if LLONG_MAX == INT64_MAX || LLONG_MAX == UINT64_MAX
    #define OWF_INT64_BITS 64
    #define OWF_LLONG_BITS 64
    #define OWF_LLONG_BYTES 8
#else
    #error "invalid LLONG_MAX value"
#endif

#if SIZE_MAX == INT32_MAX || SIZE_MAX == UINT32_MAX
    #define OWF_SIZE_BITS 32
    #define OWF_SIZE_BYTES 4
#elif SIZE_MAX == INT64_MAX || SIZE_MAX == UINT64_MAX
    #define OWF_SIZE_BITS 64
    #define OWF_SIZE_BYTES 8
#else
    #error "invalid SIZE_MAX value"
#endif

/**
 * Expect/noexpect
 */
#if OWF_PLATFORM_IS_GNU
    #define OWF_EXPECT(expr) (__builtin_expect((expr), true))
    #define OWF_NOEXPECT(expr) (__builtin_expect((expr), false))
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #define OWF_EXPECT(expr) (expr)
    #define OWF_NOEXPECT(expr) (expr)
#endif

/**
 * Bit operations
 */
#if OWF_PLATFORM_IS_GNU
    #if OWF_INT_BITS == 32
        #define OWF_CLZ_32(x) ((uint32_t)__builtin_clz((x)))
    #else
        #error "invalid OWF_INT_BITS value"
    #endif

    #if OWF_LLONG_BITS == 64
        #define OWF_CLZ_64(x) ((uint64_t)__builtin_clzll((x)))
    #else
        #error "invalid OWF_LLONG_BITS value"
    #endif
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #if OWF_INT_BITS == 32
        #define OWF_CLZ_32(x) ((uint32_t)__lzcnt((x)))
    #else
        #error "invalid OWF_INT_BITS value"
    #endif

    #if OWF_LLONG_BITS == 64
        #define OWF_CLZ_64(x) ((uint64_t)__lzcnt64((x)))
    #else
        #error "invalid OWF_INT_BITS value"
    #endif
#endif

#if OWF_SIZE_BITS == 32
    #define OWF_CLZ_SIZE(x) OWF_CLZ_32(x)
#elif OWF_SIZE_BITS == 64
    #define OWF_CLZ_SIZE(x) OWF_CLZ_64(x)
#else
    #error "invalid OWF_SIZE_BITS value"
#endif

/**
 * Token concatenation
 */
#define OWF_CONCAT2(a, b) a ## b
#define OWF_CONCAT(a, b) OWF_CONCAT2(a, b)

/**
 * Attributes
 */
#define OWF_PACK __attribute__ ((packed))
#define OWF_NORETURN __attribute__ ((noreturn))

/**
 * Array counts. From Chromium source.
 */
#define OWF_COUNT(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * Endianness
 */
#define OWF_HOST64(value) OWF_ENDIAN_SWAP64(value)
#define OWF_NET64(value) OWF_ENDIAN_SWAP64(value)
#define OWF_HOST32(value) OWF_ENDIAN_SWAP32(value)
#define OWF_NET32(value) OWF_ENDIAN_SWAP32(value)
#define OWF_HOST16(value) OWF_ENDIAN_SWAP16(value)
#define OWF_NET16(value) OWF_ENDIAN_SWAP16(value)

#define OWF_ENDIAN_LITTLE 0
#define OWF_ENDIAN_BIG 1
#define OWF_ENDIAN_NOP(value) do {value = value;} while (0)
#define OWF_ENDIAN_CAST(value, type) (*((type *)(&value)))

#if OWF_PLATFORM_IS_GNU
    #if __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define OWF_ENDIAN OWF_ENDIAN_BIG
    #elif __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define OWF_ENDIAN OWF_ENDIAN_LITTLE
    #elif __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
        #error "Please don't build libowf on a PDP."
    #else
        #error "Invalid endianness for a gnuc system"
    #endif

    #if OWF_ENDIAN == OWF_ENDIAN_BIG
        #define OWF_ENDIAN_SWAP64(value) OWF_ENDIAN_NOP(value)
        #define OWF_ENDIAN_SWAP32(value) OWF_ENDIAN_NOP(value)
        #define OWF_ENDIAN_SWAP16(value) OWF_ENDIAN_NOP(value)
    #elif OWF_ENDIAN == OWF_ENDIAN_LITTLE
        #define OWF_ENDIAN_SWAP64(value) do {OWF_ENDIAN_CAST(value, uint64_t) = __builtin_bswap64(OWF_ENDIAN_CAST(value, uint64_t));} while (0)
        #define OWF_ENDIAN_SWAP32(value) do {OWF_ENDIAN_CAST(value, uint32_t) = __builtin_bswap32(OWF_ENDIAN_CAST(value, uint32_t));} while (0)
        #define OWF_ENDIAN_SWAP16(value) do {OWF_ENDIAN_CAST(value, uint16_t) = __builtin_bswap16(OWF_ENDIAN_CAST(value, uint16_t));} while (0)
    #endif
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #define OWF_ENDIAN OWF_ENDIAN_LITTLE
    #if OWF_ENDIAN == OWF_ENDIAN_LITTLE
        #define OWF_ENDIAN_SWAP64(value) do {OWF_ENDIAN_CAST(value, uint64_t) = _byteswap_uint64(OWF_ENDIAN_CAST(value, uint64_t));} while (0)
        #define OWF_ENDIAN_SWAP32(value) do {OWF_ENDIAN_CAST(value, uint32_t) = _byteswap_ulong(OWF_ENDIAN_CAST(value, uint32_t));} while (0)
        #define OWF_ENDIAN_SWAP16(value) do {OWF_ENDIAN_CAST(value, uint16_t) = _byteswap_ushort(OWF_ENDIAN_CAST(value, uint16_t));} while (0)
    #else
        #error "Invalid endianness for a Windows system"
    #endif
#endif

/**
 * printf definitions
 */
#define OWF_PRINT_U8 "%"PRIu8
#define OWF_PRINT_S8 "%"PRId8
#define OWF_PRINT_U16 "%"PRIu16
#define OWF_PRINT_S16 "%"PRId16
#define OWF_PRINT_U32 "%"PRIu32
#define OWF_PRINT_S32 "%"PRId32
#define OWF_PRINT_U64 "%"PRIu64
#define OWF_PRINT_S64 "%"PRId64
#define OWF_PRINT_TIME OWF_PRINT_S64

#if OWF_PLATFORM_IS_GNU
    #define OWF_PRINT_SIZE  "%zu"
    #define OWF_PRINT_SSIZE "%zd"
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #define OWF_PRINT_SIZE  "%Iu"
    #define OWF_PRINT_SSIZE "%Id"
#endif

/**
 * Shims for GNU components on Windows
 */
#if OWF_PLATFORM_IS_GNU
    #include <unistd.h>
    #define owf_snprintf(...) snprintf(__VA_ARGS__)
    #define owf_vsnprintf(...) vsnprintf(__VA_ARGS__)
    #define owf_sscanf(...) sscanf(__VA_ARGS__)
    #define owf_fopen(...) fopen(__VA_ARGS__)
    #define owf_strcasecmp(...) strcasecmp(__VA_ARGS__)
    #define owf_strncasecmp(...) strncasecmp(__VA_ARGS__)
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
    #define owf_snprintf(...) owf_platform_windows_snprintf(__VA_ARGS__)
    #define owf_vsnprintf(...) owf_platform_windows_vsnprintf(__VA_ARGS__)
    #define owf_fopen(...) owf_platform_windows_fopen(__VA_ARGS__)
    #define owf_strcasecmp _stricmp
    #define owf_strncasecmp _strnicmp

    FILE *owf_platform_windows_fopen(const char *filename, const char *mode);
    int owf_platform_windows_snprintf(char *dst, size_t size, const char *format, ...);
    int owf_platform_windows_vsnprintf(char *dst, size_t size, const char *format, va_list va);
#endif

#endif /* OWF_PLATFORM_H */

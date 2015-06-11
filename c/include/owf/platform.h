#include <owf.h>

#include <stdlib.h>
#include <stdio.h>

#ifndef OWF_PLATFORM_H
#define OWF_PLATFORM_H

/** Basic platform detection */
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
#elif !defined(__GNUC__) && (defined(_WINDOWS) || defined(_WIN32))
	#define OWF_PLATFORM OWF_PLATFORM_WINDOWS
#else
	#error Don't know how to build on your platform!
#endif

#if OWF_PLATFORM == OWF_PLATFORM_LINUX || OWF_PLATFORM == OWF_PLATFORM_BSD || OWF_PLATFORM ==  OWF_PLATFORM_DARWIN || OWF_PLATFORM == OWF_PLATFORM_CYGWIN || OWF_PLATFORM == OWF_PLATFORM_MINGW
	#define OWF_PLATFORM_IS_GNU 1
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
	#define OWF_PLATFORM_IS_GNU 0
#else
	#error Don't know whether your platform is GNU or not!
#endif

/** Expect/noexpect */
#if OWF_PLATFORM_IS_GNU
	#define OWF_EXPECT(expr) (__builtin_expect((expr), true))
	#define OWF_NOEXPECT(expr) (__builtin_expect((expr), false))
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
	#define OWF_EXPECT(expr) (expr)
	#define OWF_NOEXPECT(expr) (expr)
#endif

/** Endianness */
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
    #else
        #error Invalid endianness for a gnuc system
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
        #error Invalid endianness for a Windows system
    #endif
#endif

/* Shims for stdlib _s functions on Windows */
#if OWF_PLATFORM_IS_GNU
	#include <unistd.h>
#elif OWF_PLATFORM == OWF_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <direct.h>
	#define snprintf(...) owf_platform_windows_snprintf(__VA_ARGS__)
	#define fopen(...) owf_platform_windows_fopen(__VA_ARGS__)
	#define getcwd(...) owf_platform_windows_getcwd(__VA_ARGS__)
	
	FILE *owf_platform_windows_fopen(const char *filename, const char *mode);
	int owf_platform_windows_snprintf(char *dst, size_t size, const char *format, ...);
	char *owf_platform_windows_getcwd(char *buf, size_t size);
#endif

#endif /* OWF_PLATFORM_H */

#include <owf.h>

#ifndef OWF_ENDIAN_H
#define OWF_ENDIAN_H

#define OWF_BIG_ENDIAN 1
#define OWF_LITTLE_ENDIAN 0
#define OWF_ENDIAN_NOP(value) do {value = value;} while (0)
#define OWF_ENDIAN_CAST(value, type) (*((type *)(&value)))

#if defined(__GNUC__) && (defined(__APPLE__) || defined(__linux__))
    #if __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define OWF_ENDIAN OWF_BIG_ENDIAN
    #elif __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define OWF_ENDIAN OWF_LITTLE_ENDIAN
    #else
        #error Invalid endianness for a gnuc system
    #endif

    #if OWF_ENDIAN == OWF_BIG_ENDIAN
        #define OWF_ENDIAN_SWAP64(value) OWF_ENDIAN_NOP(value)
        #define OWF_ENDIAN_SWAP32(value) OWF_ENDIAN_NOP(value)
        #define OWF_ENDIAN_SWAP16(value) OWF_ENDIAN_NOP(value)
    #elif OWF_ENDIAN == OWF_LITTLE_ENDIAN
        #define OWF_ENDIAN_SWAP64(value) do {OWF_ENDIAN_CAST(value, uint64_t) = __builtin_bswap64(OWF_ENDIAN_CAST(value, uint64_t));} while (0)
        #define OWF_ENDIAN_SWAP32(value) do {OWF_ENDIAN_CAST(value, uint32_t) = __builtin_bswap32(OWF_ENDIAN_CAST(value, uint32_t));} while (0)
        #define OWF_ENDIAN_SWAP16(value) do {OWF_ENDIAN_CAST(value, uint16_t) = __builtin_bswap16(OWF_ENDIAN_CAST(value, uint16_t));} while (0)
    #endif
#elif defined(_WINDOWS) || defined(_WIN32) || defined(__CYGWIN__)
    /* This is easy */
    #define OWF_ENDIAN OWF_LITTLE_ENDIAN
    #ifdef __CYGWIN__
        #include <byteswap.h>
        #define _byteswap_uint64 bswap_64
        #define _byteswap_ulong  bswap_32
        #define _byteswap_ushort bswap_16
    #endif

    #if OWF_ENDIAN == OWF_LITTLE_ENDIAN
        #define OWF_ENDIAN_SWAP64(value) do {OWF_ENDIAN_CAST(value, uint64_t) = _byteswap_uint64(OWF_ENDIAN_CAST(value, uint64_t));} while (0)
        #define OWF_ENDIAN_SWAP32(value) do {OWF_ENDIAN_CAST(value, uint32_t) = _byteswap_ulong(OWF_ENDIAN_CAST(value, uint32_t));} while (0)
        #define OWF_ENDIAN_SWAP16(value) do {OWF_ENDIAN_CAST(value, uint16_t) = _byteswap_ushort(OWF_ENDIAN_CAST(value, uint16_t));} while (0)
    #else
        #error Invalid endianness for a Windows system
    #endif
#else
    #error Don't know how to swap endianness on your platform
#endif

#define OWF_HOST64(value) OWF_ENDIAN_SWAP64(value)
#define OWF_NET64(value) OWF_ENDIAN_SWAP64(value)
#define OWF_HOST32(value) OWF_ENDIAN_SWAP32(value)
#define OWF_NET32(value) OWF_ENDIAN_SWAP32(value)
#define OWF_HOST16(value) OWF_ENDIAN_SWAP16(value)
#define OWF_NET16(value) OWF_ENDIAN_SWAP16(value)

#endif /* OWF_ENDIAN_H */

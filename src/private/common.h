#pragma once

// clang-format off

/**
 * Header checks
 */
#if __has_include(<sys/mman.h>)
    #define HAVE_SYS_MMAN_H 1
#endif

#if __has_include(<sys/random.h>)
    #define HAVE_SYS_RANDOM_H 1
#endif

#if __has_include(<intrin.h>)
    #define HAVE_INTRIN_H 1
#endif

#if __has_include(<sys/auxv.h>)
    #define HAVE_SYS_AUXV_H 1
#endif

/**
 * Architectural checks for intrinsics
 */
#if __has_include(<mmintrin.h>) && __MMX__
    #define HAVE_MMINTRIN_H 1
#endif

#if __has_include(<emmintrin.h>) && __SSE2__
    #define HAVE_EMMINTRIN_H 1
#endif

#if __SSE3__
    #if __has_include(<pmmintrin.h>)
        #define HAVE_PMMINTRIN_H 1
    #endif
    #if __has_include(<tmmintrin.h>)
        #define HAVE_TMMINTRIN_H 1
    #endif
#endif

#if __has_include(<smmintrin.h>) && __SSE4_1__
    #define HAVE_SMMINTRIN_H
#endif

#if __has_include(<immintrin.h>)
    #if __AVX__
        #define HAVE_AVXINTRIN_H
    #endif
    #if __AVX2__
        #define HAVE_AVX2INTRIN_H
    #endif
    #if __AVX512F__
        #if defined(__clang__) && __clang_major__ < 4
            // AVX512 may be broken
        #elif defined(__GNUC__) && __GNUC__ < 6
            // ''
        #else
            #define HAVE_AVX512FINTRIN_H
        #endif
    #endif
#endif

#if __has_include(<wmmintrin.h>) && __AES__
    #define HAVE_WMMINTRIN_H 1
#endif

#if __RDRND__
    #define HAVE_RDRAND
#endif

/**
 * Detect mman APIs
 */
#if __has_include(<sys/mman.h>)
    #define HAVE_MMAP 1
    #define HAVE_MPROTECT 1
    #define HAVE_MLOCK 1

    #if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
        #define HAVE_MADVISE 1
    #endif
#endif

#if __has_include(<sys/random.h>)
    #define HAVE_GETRANDOM 1
#endif

/**
 * POSIX-Only stuff
 */
#if __has_include(<unistd.h>)
    #if defined(_DEFAULT_SOURCE)
        #define HAVE_GETENTROPY 1
    #endif

    /**
     * Default POSIX APIs
     */
    #define HAVE_POSIX_MEMALIGN 1
    #define HAVE_GETPID 1
    #define HAVE_NANOSLEEP 1

    /**
     * Language/library features from C11
     */
    #if __STDC_VERSION__ >= 201112L
        #define HAVE_MEMSET_S 1
    #endif

    #if __linux__
        #define HAVE_EXPLICIT_BZERO 1
    #endif
#endif

/**
 * Miscellaneous
 */
#if __has_include(<pthread.h>)
    #define HAVE_PTHREAD 1
#endif

#if __has_include(<sys/param.h>)
    #include <sys/param.h>
    #if __BYTE_ORDER == __BIG_ENDIAN
        #define NATIVE_BIG_ENDIAN 1
    #elif __BYTE_ORDER == __LITTLE_ENDIAN
        #define NATIVE_LITTLE_ENDIAN 1
    #else
        #error "Unknown endianness for this platform."
    #endif
#elif defined(_MSVC)
    // At time of writing, MSVC only targets little-endian.
    #define NATIVE_LITTLE_ENDIAN 1
#else
    #error "Unknown endianness for this platform."
#endif

#define CONFIGURED 1

#ifndef common_H
#define common_H 1

#if !defined(_MSC_VER) && 0
# warning *** This is unstable, untested, development code.
# warning It might not compile. It might not work as expected.
# warning It might be totally insecure.
# warning Do not use this except if you are planning to contribute code.
# warning Use releases available at https://download.libsodium.org/libsodium/releases/ instead.
# warning Alternatively, use the "stable" branch in the git repository.
#endif

#if !defined(_MSC_VER) && (!defined(CONFIGURED) || CONFIGURED != 1)
# warning *** The library is being compiled using an undocumented method.
# warning This is not supported. It has not been tested, it might not
# warning work as expected, and performance is likely to be suboptimal.
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define COMPILER_ASSERT(X) (void) sizeof(char[(X) ? 1 : -1])

#ifdef HAVE_TI_MODE
# if defined(__SIZEOF_INT128__)
typedef unsigned __int128 uint128_t;
# else
typedef unsigned uint128_t __attribute__((mode(TI)));
# endif
#endif

#define ROTL32(X, B) rotl32((X), (B))
static inline uint32_t
rotl32(const uint32_t x, const int b)
{
    return (x << b) | (x >> (32 - b));
}

#define ROTL64(X, B) rotl64((X), (B))
static inline uint64_t
rotl64(const uint64_t x, const int b)
{
    return (x << b) | (x >> (64 - b));
}

#define ROTR32(X, B) rotr32((X), (B))
static inline uint32_t
rotr32(const uint32_t x, const int b)
{
    return (x >> b) | (x << (32 - b));
}

#define ROTR64(X, B) rotr64((X), (B))
static inline uint64_t
rotr64(const uint64_t x, const int b)
{
    return (x >> b) | (x << (64 - b));
}

#define LOAD64_LE(SRC) load64_le(SRC)
static inline uint64_t
load64_le(const uint8_t src[8])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint64_t w;
    memcpy(&w, src, sizeof w);
    return w;
#else
    uint64_t w = (uint64_t) src[0];
    w |= (uint64_t) src[1] <<  8;
    w |= (uint64_t) src[2] << 16;
    w |= (uint64_t) src[3] << 24;
    w |= (uint64_t) src[4] << 32;
    w |= (uint64_t) src[5] << 40;
    w |= (uint64_t) src[6] << 48;
    w |= (uint64_t) src[7] << 56;
    return w;
#endif
}

#define STORE64_LE(DST, W) store64_le((DST), (W))
static inline void
store64_le(uint8_t dst[8], uint64_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    memcpy(dst, &w, sizeof w);
#else
    dst[0] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w; w >>= 8;
    dst[4] = (uint8_t) w; w >>= 8;
    dst[5] = (uint8_t) w; w >>= 8;
    dst[6] = (uint8_t) w; w >>= 8;
    dst[7] = (uint8_t) w;
#endif
}

#define LOAD32_LE(SRC) load32_le(SRC)
static inline uint32_t
load32_le(const uint8_t src[4])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint32_t w;
    memcpy(&w, src, sizeof w);
    return w;
#else
    uint32_t w = (uint32_t) src[0];
    w |= (uint32_t) src[1] <<  8;
    w |= (uint32_t) src[2] << 16;
    w |= (uint32_t) src[3] << 24;
    return w;
#endif
}

#define STORE32_LE(DST, W) store32_le((DST), (W))
static inline void
store32_le(uint8_t dst[4], uint32_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    memcpy(dst, &w, sizeof w);
#else
    dst[0] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w;
#endif
}

/* ----- */

#define LOAD64_BE(SRC) load64_be(SRC)
static inline uint64_t
load64_be(const uint8_t src[8])
{
#ifdef NATIVE_BIG_ENDIAN
    uint64_t w;
    memcpy(&w, src, sizeof w);
    return w;
#else
    uint64_t w = (uint64_t) src[7];
    w |= (uint64_t) src[6] <<  8;
    w |= (uint64_t) src[5] << 16;
    w |= (uint64_t) src[4] << 24;
    w |= (uint64_t) src[3] << 32;
    w |= (uint64_t) src[2] << 40;
    w |= (uint64_t) src[1] << 48;
    w |= (uint64_t) src[0] << 56;
    return w;
#endif
}

#define STORE64_BE(DST, W) store64_be((DST), (W))
static inline void
store64_be(uint8_t dst[8], uint64_t w)
{
#ifdef NATIVE_BIG_ENDIAN
    memcpy(dst, &w, sizeof w);
#else
    dst[7] = (uint8_t) w; w >>= 8;
    dst[6] = (uint8_t) w; w >>= 8;
    dst[5] = (uint8_t) w; w >>= 8;
    dst[4] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[0] = (uint8_t) w;
#endif
}

#define LOAD32_BE(SRC) load32_be(SRC)
static inline uint32_t
load32_be(const uint8_t src[4])
{
#ifdef NATIVE_BIG_ENDIAN
    uint32_t w;
    memcpy(&w, src, sizeof w);
    return w;
#else
    uint32_t w = (uint32_t) src[3];
    w |= (uint32_t) src[2] <<  8;
    w |= (uint32_t) src[1] << 16;
    w |= (uint32_t) src[0] << 24;
    return w;
#endif
}

#define STORE32_BE(DST, W) store32_be((DST), (W))
static inline void
store32_be(uint8_t dst[4], uint32_t w)
{
#ifdef NATIVE_BIG_ENDIAN
    memcpy(dst, &w, sizeof w);
#else
    dst[3] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[0] = (uint8_t) w;
#endif
}

#define XOR_BUF(OUT, IN, N) xor_buf((OUT), (IN), (N))
static inline void
xor_buf(unsigned char *out, const unsigned char *in, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        out[i] ^= in[i];
    }
}

#if !defined(__clang__) && !defined(__GNUC__)
# ifdef __attribute__
#  undef __attribute__
# endif
# define __attribute__(a)
#endif

#ifndef CRYPTO_ALIGN
# if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#  define CRYPTO_ALIGN(x) __declspec(align(x))
# else
#  define CRYPTO_ALIGN(x) __attribute__ ((aligned(x)))
# endif
#endif

#if defined(_MSC_VER) && \
    (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))

# include <intrin.h>

# define HAVE_INTRIN_H    1
# define HAVE_MMINTRIN_H  1
# define HAVE_EMMINTRIN_H 1
# define HAVE_PMMINTRIN_H 1
# define HAVE_TMMINTRIN_H 1
# define HAVE_SMMINTRIN_H 1
# define HAVE_AVXINTRIN_H 1
# if _MSC_VER >= 1600
#  define HAVE_WMMINTRIN_H 1
# endif
# if _MSC_VER >= 1700 && defined(_M_X64)
#  define HAVE_AVX2INTRIN_H 1
# endif
#elif defined(HAVE_INTRIN_H)
# include <intrin.h>
#endif

#ifdef HAVE_LIBCTGRIND
extern void ct_poison  (const void *, size_t);
extern void ct_unpoison(const void *, size_t);
# define POISON(X, L)   ct_poison((X), (L))
# define UNPOISON(X, L) ct_unpoison((X), (L))
#else
# define POISON(X, L)   (void) 0
# define UNPOISON(X, L) (void) 0
#endif

#endif

#ifndef BOTAN_BUILD_CONFIG_H__
#define BOTAN_BUILD_CONFIG_H__

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 11
#define BOTAN_VERSION_PATCH 13
#define BOTAN_VERSION_DATESTAMP 20150111

#define BOTAN_VERSION_RELEASE_TYPE "released"

#define BOTAN_VERSION_VC_REVISION "mtn:ea1f16ee8c0bb5549c427a71d277e60c924488e3"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#ifndef BOTAN_DLL
  #define BOTAN_DLL
#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Maximum size to allocate out of the mlock pool */
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 32

/* PK key consistency checking toggles */
#define BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD 1
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD 0
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE 1

/*
* RNGs will automatically poll the system for additional
* seed material after producing this many bytes of output.
*/
#define BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED 512
#define BOTAN_RNG_RESEED_POLL_BITS 128

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && defined(__GNUG__)
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #define BOTAN_USE_GCC_INLINE_ASM 0
#endif

#ifdef __GNUC__
  #define BOTAN_GCC_VERSION \
     (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/* Target identification and feature test macros */

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

#if defined(_MSC_VER)
  // 4250: inherits via dominance (diamond inheritence issue)
  // 4251: needs DLL interface (STL DLL exports)
  #pragma warning(disable: 4250 4251)
#endif

/*
* Compile-time deprecatation warnings
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)

    #if BOTAN_GCC_VERSION >= 450
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
    #else
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))
    #endif

  #endif

#endif

#if defined(_MSC_VER)
  #define BOTAN_CURRENT_FUNCTION __FUNCTION__
  #define noexcept
#else
  #define BOTAN_CURRENT_FUNCTION __func__
#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_ADLER32
#define BOTAN_HAS_AEAD_CCM
#define BOTAN_HAS_AEAD_CHACHA20_POLY1305
#define BOTAN_HAS_AEAD_EAX
#define BOTAN_HAS_AEAD_FILTER
#define BOTAN_HAS_AEAD_GCM
#define BOTAN_HAS_AEAD_OCB
#define BOTAN_HAS_AEAD_SIV
#define BOTAN_HAS_AES
#define BOTAN_HAS_ANSI_X919_MAC
#define BOTAN_HAS_BLOWFISH
#define BOTAN_HAS_CAMELLIA
#define BOTAN_HAS_CASCADE
#define BOTAN_HAS_CAST
#define BOTAN_HAS_CBC_MAC
#define BOTAN_HAS_CHACHA
#define BOTAN_HAS_CIPHER_MODE_PADDING
#define BOTAN_HAS_CMAC
#define BOTAN_HAS_CODEC_FILTERS
#define BOTAN_HAS_COMB4P
#define BOTAN_HAS_CRC24
#define BOTAN_HAS_CRC32
#define BOTAN_HAS_CTR_BE
#define BOTAN_HAS_CURVE_25519
#define BOTAN_HAS_DES
#define BOTAN_HAS_DIFFIE_HELLMAN
#define BOTAN_HAS_DSA
#define BOTAN_HAS_ECDH
#define BOTAN_HAS_ECDSA
#define BOTAN_HAS_ELGAMAL
#define BOTAN_HAS_EME_OAEP
#define BOTAN_HAS_EME_PKCS1v15
#define BOTAN_HAS_EMSA1
#define BOTAN_HAS_EMSA1_BSI
#define BOTAN_HAS_EMSA_PKCS1
#define BOTAN_HAS_EMSA_PSSR
#define BOTAN_HAS_EMSA_RAW
#define BOTAN_HAS_EMSA_X931
#define BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER
#define BOTAN_HAS_ENTROPY_SRC_RDRAND
#define BOTAN_HAS_GOST_28147_89
#define BOTAN_HAS_GOST_34_10_2001
#define BOTAN_HAS_GOST_34_11
#define BOTAN_HAS_HAS_160
#define BOTAN_HAS_HMAC
#define BOTAN_HAS_IDEA
#define BOTAN_HAS_KASUMI
#define BOTAN_HAS_KDF1
#define BOTAN_HAS_KDF2
#define BOTAN_HAS_KECCAK
#define BOTAN_HAS_LION
#define BOTAN_HAS_MARS
#define BOTAN_HAS_MD2
#define BOTAN_HAS_MD4
#define BOTAN_HAS_MD5
#define BOTAN_HAS_MISTY1
#define BOTAN_HAS_MODE_CBC
#define BOTAN_HAS_MODE_CFB
#define BOTAN_HAS_MODE_ECB
#define BOTAN_HAS_MODE_XTS
#define BOTAN_HAS_NOEKEON
#define BOTAN_HAS_NYBERG_RUEPPEL
#define BOTAN_HAS_OFB
#define BOTAN_HAS_PARALLEL_HASH
#define BOTAN_HAS_PBKDF1
#define BOTAN_HAS_PBKDF2
#define BOTAN_HAS_POLY1305
#define BOTAN_HAS_RC2
#define BOTAN_HAS_RC4
#define BOTAN_HAS_RC5
#define BOTAN_HAS_RC6
#define BOTAN_HAS_RIPEMD_128
#define BOTAN_HAS_RIPEMD_160
#define BOTAN_HAS_RSA
#define BOTAN_HAS_RW
#define BOTAN_HAS_SAFER
#define BOTAN_HAS_SALSA20
#define BOTAN_HAS_SEED
#define BOTAN_HAS_SERPENT
#define BOTAN_HAS_SHA1
#define BOTAN_HAS_SHA2_32
#define BOTAN_HAS_SHA2_64
#define BOTAN_HAS_SIPHASH
#define BOTAN_HAS_SKEIN_512
#define BOTAN_HAS_TEA
#define BOTAN_HAS_THREEFISH_512
#define BOTAN_HAS_TIGER
#define BOTAN_HAS_TLS
#define BOTAN_HAS_TLS_V10_PRF
#define BOTAN_HAS_TLS_V10_PRF
#define BOTAN_HAS_TLS_V12_PRF
#define BOTAN_HAS_TWOFISH
#define BOTAN_HAS_WHIRLPOOL
#define BOTAN_HAS_X942_PRF
#define BOTAN_HAS_XTEA


/*
* Local configuration options (if any) follow
*/

#endif

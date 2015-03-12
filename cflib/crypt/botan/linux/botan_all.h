/*
* Botan 1.11.15 Amalgamation
* (C) 1999-2013,2014,2015 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_H__
#define BOTAN_AMALGAMATION_H__

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <initializer_list>
#include <iosfwd>
#include <istream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/*
* This file was automatically generated Thu Mar 12 11:50:47 2015 UTC by
* fishbach@cadvitecdev running './configure.py'
*
* Target
*  - Compiler: g++ -m64 -pthread -fstack-protector -O2 -momit-leaf-frame-pointer
*  - Arch: x86_64/x86_64
*  - OS: linux
*/

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 11
#define BOTAN_VERSION_PATCH 15
#define BOTAN_VERSION_DATESTAMP 20150308

#define BOTAN_VERSION_RELEASE_TYPE "released"

#define BOTAN_VERSION_VC_REVISION "mtn:89d930531d07db67637637ab60a1e9d8ac71c3e5"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#define BOTAN_INSTALL_PREFIX "/usr/local"
#define BOTAN_INSTALL_HEADER_DIR "include/botan-1.11"
#define BOTAN_INSTALL_LIB_DIR "lib"
#define BOTAN_LIB_LINK "-lrt"

#ifndef BOTAN_DLL
  #define BOTAN_DLL __attribute__((visibility("default")))
#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Maximum size to allocate out of the mlock pool */
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 64

/*
If enabled uses memset via volatile function pointer to zero memory,
otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/*
* If enabled the ECC implementation will use Montgomery ladder
* instead of a fixed window implementation.
*/
#define BOTAN_CURVE_GFP_USE_MONTGOMERY_LADDER 0

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
#define BOTAN_TARGET_OS_IS_LINUX
#define BOTAN_TARGET_OS_HAS_CLOCK_GETTIME
#define BOTAN_TARGET_OS_HAS_DLOPEN
#define BOTAN_TARGET_OS_HAS_GETTIMEOFDAY
#define BOTAN_TARGET_OS_HAS_GMTIME_R
#define BOTAN_TARGET_OS_HAS_POSIX_MLOCK
#define BOTAN_TARGET_OS_HAS_READDIR

#define BOTAN_TARGET_ARCH_IS_X86_64
#define BOTAN_TARGET_SUPPORTS_AESNI
#define BOTAN_TARGET_SUPPORTS_AVX2
#define BOTAN_TARGET_SUPPORTS_BMI2
#define BOTAN_TARGET_SUPPORTS_CLMUL
#define BOTAN_TARGET_SUPPORTS_RDRAND
#define BOTAN_TARGET_SUPPORTS_SHA
#define BOTAN_TARGET_SUPPORTS_SSE2
#define BOTAN_TARGET_SUPPORTS_SSE41
#define BOTAN_TARGET_SUPPORTS_SSE42
#define BOTAN_TARGET_SUPPORTS_SSSE3
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
#define BOTAN_TARGET_CPU_IS_X86_FAMILY
#define BOTAN_TARGET_CPU_NATIVE_WORD_SIZE 64
#define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 1

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

#define BOTAN_BUILD_COMPILER_IS_GCC

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
#else
  #define BOTAN_CURRENT_FUNCTION __func__
#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

#if defined(_MSC_VER)
  // noexcept is not supported in VS 2013
  #include <yvals.h>
  #define BOTAN_NOEXCEPT _NOEXCEPT
#else
  #define BOTAN_NOEXCEPT noexcept
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_ADLER32 20131128
#define BOTAN_HAS_AEAD_CCM 20131128
#define BOTAN_HAS_AEAD_CHACHA20_POLY1305 20141228
#define BOTAN_HAS_AEAD_EAX 20131128
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AEAD_OCB 20131128
#define BOTAN_HAS_AEAD_SIV 20131202
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_AES_NI 20131128
#define BOTAN_HAS_AES_SSSE3 20131128
#define BOTAN_HAS_ANSI_X919_MAC 20131128
#define BOTAN_HAS_ASN1 20131128
#define BOTAN_HAS_AUTO_SEEDING_RNG 20131128
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BCRYPT 20131128
#define BOTAN_HAS_BIGINT 20131128
#define BOTAN_HAS_BIGINT_MATH 20131128
#define BOTAN_HAS_BIGINT_MP 20131128
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_BLOWFISH 20131128
#define BOTAN_HAS_CAMELLIA 20131128
#define BOTAN_HAS_CASCADE 20131128
#define BOTAN_HAS_CAST 20131128
#define BOTAN_HAS_CBC_MAC 20131128
#define BOTAN_HAS_CHACHA 20140103
#define BOTAN_HAS_CIPHER_MODE_PADDING 20131128
#define BOTAN_HAS_CMAC 20131128
#define BOTAN_HAS_CODEC_FILTERS 20131128
#define BOTAN_HAS_COMB4P 20131128
#define BOTAN_HAS_COMPRESSION 20141117
#define BOTAN_HAS_CRC24 20131128
#define BOTAN_HAS_CRC32 20131128
#define BOTAN_HAS_CREDENTIALS_MANAGER 20131128
#define BOTAN_HAS_CRYPTO_BOX 20131128
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_CURVE_25519 20141227
#define BOTAN_HAS_DES 20131128
#define BOTAN_HAS_DIFFIE_HELLMAN 20131128
#define BOTAN_HAS_DLIES 20131128
#define BOTAN_HAS_DL_GROUP 20131128
#define BOTAN_HAS_DL_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_DSA 20131128
#define BOTAN_HAS_ECC_GROUP 20131128
#define BOTAN_HAS_ECC_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_ECDH 20131128
#define BOTAN_HAS_ECDSA 20131128
#define BOTAN_HAS_EC_CURVE_GFP 20131128
#define BOTAN_HAS_ELGAMAL 20131128
#define BOTAN_HAS_EME_OAEP 20140118
#define BOTAN_HAS_EME_PKCS1v15 20131128
#define BOTAN_HAS_EMSA1 20131128
#define BOTAN_HAS_EMSA1_BSI 20131128
#define BOTAN_HAS_EMSA_PKCS1 20140118
#define BOTAN_HAS_EMSA_PSSR 20131128
#define BOTAN_HAS_EMSA_RAW 20131128
#define BOTAN_HAS_EMSA_X931 20140118
#define BOTAN_HAS_ENTROPY_SOURCE 20150201
#define BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 20131128
#define BOTAN_HAS_ENTROPY_SRC_EGD 20131128
#define BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER 20131128
#define BOTAN_HAS_ENTROPY_SRC_PROC_WALKER 20131128
#define BOTAN_HAS_ENTROPY_SRC_RDRAND 20131128
#define BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER 20131128
#define BOTAN_HAS_FFI 20150210
#define BOTAN_HAS_FILTERS 20131128
#define BOTAN_HAS_FPE_FE1 20131128
#define BOTAN_HAS_GOST_28147_89 20131128
#define BOTAN_HAS_GOST_34_10_2001 20131128
#define BOTAN_HAS_GOST_34_11 20131128
#define BOTAN_HAS_HASH_ID 20131128
#define BOTAN_HAS_HAS_160 20131128
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HKDF 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_DRBG 20140319
#define BOTAN_HAS_HMAC_RNG 20131128
#define BOTAN_HAS_HTTP_UTIL 20131128
#define BOTAN_HAS_IDEA 20131128
#define BOTAN_HAS_IDEA_SSE2 20131128
#define BOTAN_HAS_IF_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_KASUMI 20131128
#define BOTAN_HAS_KDF1 20131128
#define BOTAN_HAS_KDF2 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KECCAK 20131128
#define BOTAN_HAS_KEYPAIR_TESTING 20131128
#define BOTAN_HAS_LION 20131128
#define BOTAN_HAS_LOCKING_ALLOCATOR 20131128
#define BOTAN_HAS_MARS 20131128
#define BOTAN_HAS_MCELIECE 20141124
#define BOTAN_HAS_MD2 20131128
#define BOTAN_HAS_MD4 20131128
#define BOTAN_HAS_MD5 20131128
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MGF1 20140118
#define BOTAN_HAS_MISTY1 20131128
#define BOTAN_HAS_MODE_CBC 20131128
#define BOTAN_HAS_MODE_CFB 20131128
#define BOTAN_HAS_MODE_ECB 20131128
#define BOTAN_HAS_MODE_XTS 20131128
#define BOTAN_HAS_NOEKEON 20131128
#define BOTAN_HAS_NOEKEON_SIMD 20131128
#define BOTAN_HAS_NYBERG_RUEPPEL 20131128
#define BOTAN_HAS_OCSP 20131128
#define BOTAN_HAS_OFB 20131128
#define BOTAN_HAS_OID_LOOKUP 20131128
#define BOTAN_HAS_OPENPGP_CODEC 20131128
#define BOTAN_HAS_PACKAGE_TRANSFORM 20131128
#define BOTAN_HAS_PARALLEL_HASH 20131128
#define BOTAN_HAS_PASSHASH9 20131128
#define BOTAN_HAS_PBKDF1 20131128
#define BOTAN_HAS_PBKDF2 20131128
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PIPE_UNIXFD_IO 20131128
#define BOTAN_HAS_PKCS5_PBES2 20141119
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_POLY1305 20141227
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RC2 20131128
#define BOTAN_HAS_RC4 20131128
#define BOTAN_HAS_RC5 20131128
#define BOTAN_HAS_RC6 20131128
#define BOTAN_HAS_RFC3394_KEYWRAP 20131128
#define BOTAN_HAS_RFC6979_GENERATOR 20140321
#define BOTAN_HAS_RIPEMD_128 20131128
#define BOTAN_HAS_RIPEMD_160 20131128
#define BOTAN_HAS_RSA 20131128
#define BOTAN_HAS_RUNTIME_BENCHMARKING 20131128
#define BOTAN_HAS_RW 20131128
#define BOTAN_HAS_SAFER 20131128
#define BOTAN_HAS_SALSA20 20131128
#define BOTAN_HAS_SEED 20131128
#define BOTAN_HAS_SERPENT 20131128
#define BOTAN_HAS_SERPENT_SIMD 20131128
#define BOTAN_HAS_SHA1 20131128
#define BOTAN_HAS_SHA1_SSE2 20131128
#define BOTAN_HAS_SHA1_X86_64 20131128
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SIMD_32 20131128
#define BOTAN_HAS_SIMD_SSE2 20131128
#define BOTAN_HAS_SIPHASH 20150110
#define BOTAN_HAS_SKEIN_512 20131128
#define BOTAN_HAS_SRP6 20131128
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_SYSTEM_RNG 20141202
#define BOTAN_HAS_TEA 20131128
#define BOTAN_HAS_THREEFISH_512 20131224
#define BOTAN_HAS_THREEFISH_512_AVX2 20131224
#define BOTAN_HAS_THRESHOLD_SECRET_SHARING 20131128
#define BOTAN_HAS_TIGER 20131128
#define BOTAN_HAS_TLS 20131128
#define BOTAN_HAS_TLS_SESSION_MANAGER_SQL_DB 20141219
#define BOTAN_HAS_TLS_V10_PRF 20131128
#define BOTAN_HAS_TLS_V12_PRF 20131128
#define BOTAN_HAS_TRANSFORM 20131209
#define BOTAN_HAS_TWOFISH 20131128
#define BOTAN_HAS_UTIL_FUNCTIONS 20140123
#define BOTAN_HAS_WHIRLPOOL 20131128
#define BOTAN_HAS_X509_CERTIFICATES 20131128
#define BOTAN_HAS_X931_RNG 20131128
#define BOTAN_HAS_X942_PRF 20131128
#define BOTAN_HAS_XTEA 20131128
#define BOTAN_HAS_XTEA_SIMD 20131128

/*
* Local configuration options (if any) follow
*/



namespace Botan {

/**
* Called when an assertion fails
*/
void BOTAN_DLL assertion_failure(const char* expr_str,
                                 const char* assertion_made,
                                 const char* func,
                                 const char* file,
                                 int line);

/**
* Make an assertion
*/
#define BOTAN_ASSERT(expr, assertion_made)                \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  assertion_made,         \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Assert that value1 == value2
*/
#define BOTAN_ASSERT_EQUAL(expr1, expr2, assertion_made)   \
   do {                                                    \
     if((expr1) != (expr2))                                \
       Botan::assertion_failure(#expr1 " == " #expr2,      \
                                  assertion_made,          \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that expr1 (if true) implies expr2 is also true
*/
#define BOTAN_ASSERT_IMPLICATION(expr1, expr2, msg)        \
   do {                                                    \
     if((expr1) && !(expr2))                               \
       Botan::assertion_failure(#expr1 " implies " #expr2, \
                                msg,                       \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that a pointer is not null
*/
#define BOTAN_ASSERT_NONNULL(ptr)                          \
   do {                                                    \
      if(static_cast<bool>(ptr) == false)                  \
         Botan::assertion_failure(#ptr " is not null",     \
                                  "",                      \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Mark variable as unused
*/
#define BOTAN_UNUSED(v) static_cast<void>(v)

}

#include <stdint.h>

/**
* The primary namespace for the botan library
*/
namespace Botan {

using ::uint8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;
using ::int32_t;
using ::int64_t;

using ::size_t;

typedef uint8_t byte;
typedef uint16_t u16bit;
typedef uint32_t u32bit;
typedef uint64_t u64bit;

typedef int32_t s32bit;

/**
* A default buffer size; typically a memory page
*/
static const size_t DEFAULT_BUFFERSIZE = BOTAN_DEFAULT_BUFFER_SIZE;

}

namespace Botan_types {

using Botan::byte;
using Botan::u32bit;

}


namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class BOTAN_DLL SCAN_Name
   {
   public:
      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(const char* algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(std::string algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(std::string algo_spec, const std::string& extra);

      /**
      * @return original input string
      */
      const std::string& as_string() const { return orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return alg_name; }

      /**
      * @return algorithm name plus any arguments
      */
      std::string algo_name_and_args() const { return algo_name() + all_arguments(); }

      /**
      * @return all arguments
      */
      std::string all_arguments() const;

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const
         { return ((arg_count() >= lower) && (arg_count() <= upper)); }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, const std::string& def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const
         { return (mode_info.size() >= 1) ? mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const
         { return (mode_info.size() >= 2) ? mode_info[1] : ""; }

      static void add_alias(const std::string& alias, const std::string& basename);

      static std::string deref_alias(const std::string& alias);
   private:
      static std::mutex g_alias_map_mutex;
      static std::map<std::string, std::string> g_alias_map;

      std::string orig_algo_spec;
      std::string alg_name;
      std::vector<std::string> args;
      std::vector<std::string> mode_info;
   };

}


namespace Botan {

/**
* Zeroize memory
* @param ptr a pointer to memory to zero out
* @param n the number of bytes pointed to by ptr
*/
BOTAN_DLL void zero_mem(void* ptr, size_t n);

/**
* Zeroize memory
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   std::memset(ptr, 0, sizeof(T)*n);
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   std::memmove(out, in, sizeof(T)*n);
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, byte val)
   {
   std::memset(ptr, val, sizeof(T)*n);
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

}


#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)

namespace Botan {

class BOTAN_DLL mlock_allocator
   {
   public:
      static mlock_allocator& instance();

      void* allocate(size_t num_elems, size_t elem_size);

      bool deallocate(void* p, size_t num_elems, size_t elem_size);

      mlock_allocator(const mlock_allocator&) = delete;

      mlock_allocator& operator=(const mlock_allocator&) = delete;

   private:
      mlock_allocator();

      ~mlock_allocator();

      const size_t m_poolsize;

      std::mutex m_mutex;
      std::vector<std::pair<size_t, size_t>> m_freelist;
      byte* m_pool;
   };

}

#endif

namespace Botan {

template<typename T>
class secure_allocator
   {
   public:
      typedef T          value_type;

      typedef T*         pointer;
      typedef const T*   const_pointer;

      typedef T&         reference;
      typedef const T&   const_reference;

      typedef std::size_t     size_type;
      typedef std::ptrdiff_t  difference_type;

      secure_allocator() BOTAN_NOEXCEPT {}

      ~secure_allocator() BOTAN_NOEXCEPT {}

      pointer address(reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      const_pointer address(const_reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      pointer allocate(size_type n, const void* = 0)
         {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(pointer p = static_cast<pointer>(mlock_allocator::instance().allocate(n, sizeof(T))))
            return p;
#endif

         pointer p = new T[n];
         clear_mem(p, n);
         return p;
         }

      void deallocate(pointer p, size_type n)
         {
         zero_mem(p, n);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(mlock_allocator::instance().deallocate(p, n, sizeof(T)))
            return;
#endif

         delete [] p;
         }

      size_type max_size() const BOTAN_NOEXCEPT
         {
         return static_cast<size_type>(-1) / sizeof(T);
         }

      template<typename U, typename... Args>
      void construct(U* p, Args&&... args)
         {
         ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
         }

      template<typename U> void destroy(U* p) { p->~U(); }
   };

template<typename T> inline bool
operator==(const secure_allocator<T>&, const secure_allocator<T>&)
   { return true; }

template<typename T> inline bool
operator!=(const secure_allocator<T>&, const secure_allocator<T>&)
   { return false; }

template<typename T> using secure_vector = std::vector<T, secure_allocator<T>>;
template<typename T> using secure_deque = std::deque<T, secure_allocator<T>>;

template<typename T>
std::vector<T> unlock(const secure_vector<T>& in)
   {
   std::vector<T> out(in.size());
   copy_mem(&out[0], &in[0], in.size());
   return out;
   }

template<typename T, typename Alloc>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const T input[],
                     size_t input_length)
   {
   const size_t to_copy = std::min(input_length, buf.size() - buf_offset);
   copy_mem(&buf[buf_offset], input, to_copy);
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const std::vector<T, Alloc2>& input)
   {
   const size_t to_copy = std::min(input.size(), buf.size() - buf_offset);
   copy_mem(&buf[buf_offset], &input[0], to_copy);
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator+=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.size());
   copy_mem(&out[copy_offset], &in[0], in.size());
   return out;
   }

template<typename T, typename Alloc>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out, T in)
   {
   out.push_back(in);
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<const T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   copy_mem(&out[copy_offset], in.first, in.second);
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   copy_mem(&out[copy_offset], in.first, in.second);
   return out;
   }

/**
* Zeroise the values; length remains unchanged
* @param vec the vector to zeroise
*/
template<typename T, typename Alloc>
void zeroise(std::vector<T, Alloc>& vec)
   {
   clear_mem(&vec[0], vec.size());
   }

/**
* Zeroise the values then free the memory
* @param vec the vector to zeroise and free
*/
template<typename T, typename Alloc>
void zap(std::vector<T, Alloc>& vec)
   {
   zeroise(vec);
   vec.clear();
   vec.shrink_to_fit();
   }

}


namespace Botan {

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<typename T> inline byte get_byte(size_t byte_num, T input)
   {
   return static_cast<byte>(
      input >> ((sizeof(T)-1-(byte_num&(sizeof(T)-1))) << 3)
      );
   }

}


namespace Botan {

/**
* This class represents any kind of computation which uses an internal
* state, such as hash functions or MACs
*/
class BOTAN_DLL Buffered_Computation
   {
   public:
      /**
      * @return length of the output of this function in bytes
      */
      virtual size_t output_length() const = 0;

      /**
      * Add new input to process.
      * @param in the input to process as a byte array
      * @param length of param in in bytes
      */
      void update(const byte in[], size_t length) { add_data(in, length); }

      /**
      * Add new input to process.
      * @param in the input to process as a secure_vector
      */
      void update(const secure_vector<byte>& in)
         {
         add_data(&in[0], in.size());
         }

      /**
      * Add new input to process.
      * @param in the input to process as a std::vector
      */
      void update(const std::vector<byte>& in)
         {
         add_data(&in[0], in.size());
         }

      /**
      * Add an integer in big-endian order
      * @param in the value
      */
      template<typename T> void update_be(const T in)
         {
         for(size_t i = 0; i != sizeof(T); ++i)
            {
            byte b = get_byte(i, in);
            add_data(&b, 1);
            }
         }

      /**
      * Add new input to process.
      * @param str the input to process as a std::string. Will be interpreted
      * as a byte array based on
      * the strings encoding.
      */
      void update(const std::string& str)
         {
         add_data(reinterpret_cast<const byte*>(str.data()), str.size());
         }

      /**
      * Process a single byte.
      * @param in the byte to process
      */
      void update(byte in) { add_data(&in, 1); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @param out The byte array to be filled with the result.
      * Must be of length output_length()
      */
      void final(byte out[]) { final_result(out); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @return secure_vector holding the result
      */
      secure_vector<byte> final()
         {
         secure_vector<byte> output(output_length());
         final_result(&output[0]);
         return output;
         }

      template<typename Alloc>
         void final(std::vector<byte, Alloc>& out)
         {
         out.resize(output_length());
         final_result(&out[0]);
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a byte array
      * @param length the length of the byte array
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const byte in[], size_t length)
         {
         add_data(in, length);
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const secure_vector<byte>& in)
         {
         add_data(&in[0], in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::vector<byte>& in)
         {
         add_data(&in[0], in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a string
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::string& in)
         {
         update(in);
         return final();
         }

      virtual ~Buffered_Computation() {}
   private:
      /**
      * Add more data to the computation
      * @param input is an input buffer
      * @param length is the length of input in bytes
      */
      virtual void add_data(const byte input[], size_t length) = 0;

      /**
      * Write the final output to out
      * @param out is an output buffer of output_length()
      */
      virtual void final_result(byte out[]) = 0;
   };

}


namespace Botan {

/**
* This class represents hash function (message digest) objects
*/
class BOTAN_DLL HashFunction : public Buffered_Computation
   {
   public:
      /**
      * @return new object representing the same algorithm as *this
      */
      virtual HashFunction* clone() const = 0;

      virtual void clear() = 0;

      virtual std::string name() const = 0;

      /**
      * @return hash block size as defined for this algorithm
      */
      virtual size_t hash_block_size() const { return 0; }

      typedef SCAN_Name Spec;
   };

}


namespace Botan {

/**
* The Adler32 checksum, used in zlib
*/
class BOTAN_DLL Adler32 : public HashFunction
   {
   public:
      std::string name() const { return "Adler32"; }
      size_t output_length() const { return 4; }
      HashFunction* clone() const { return new Adler32; }

      void clear() { S1 = 1; S2 = 0; }

      Adler32() { clear(); }
      ~Adler32() { clear(); }
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      u16bit S1, S2;
   };

}


namespace Botan {

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_DLL Key_Length_Specification
   {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      Key_Length_Specification(size_t keylen) :
         min_keylen(keylen),
         max_keylen(keylen),
         keylen_mod(1)
         {
         }

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k,
                               size_t max_k,
                               size_t k_mod = 1) :
         min_keylen(min_k),
         max_keylen(max_k ? max_k : min_k),
         keylen_mod(k_mod)
         {
         }

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const
         {
         return ((length >= min_keylen) &&
                 (length <= max_keylen) &&
                 (length % keylen_mod == 0));
         }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const
         {
         return min_keylen;
         }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const
         {
         return max_keylen;
         }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const
         {
         return keylen_mod;
         }

      Key_Length_Specification multiple(size_t n) const
         {
         return Key_Length_Specification(n * min_keylen,
                                         n * max_keylen,
                                         n * keylen_mod);
         }

   private:
      size_t min_keylen, max_keylen, keylen_mod;
   };

}



namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
BOTAN_DLL std::vector<std::string>
parse_algorithm_name(const std::string& scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimitor
* @return string split by delim
*/
BOTAN_DLL std::vector<std::string> split_on(
   const std::string& str, char delim);

/**
* Split a string on a character predicate
* @param str the input string
*/
BOTAN_DLL std::vector<std::string>
split_on_pred(const std::string& str,
              std::function<bool (char)> pred);

/**
* Erase characters from a string
*/
BOTAN_DLL std::string erase_chars(const std::string& str, const std::set<char>& chars);

/**
* Replace a character in a string
* @param str the input string
* @param from_char the character to replace
* @param to_char the character to replace it with
* @return str with all instances of from_char replaced by to_char
*/
BOTAN_DLL std::string replace_char(const std::string& str,
                                   char from_char,
                                   char to_char);

/**
* Replace a character in a string
* @param str the input string
* @param from_chars the characters to replace
* @param to_char the character to replace it with
* @return str with all instances of from_chars replaced by to_char
*/
BOTAN_DLL std::string replace_chars(const std::string& str,
                                    const std::set<char>& from_chars,
                                    char to_char);

/**
* Join a string
* @param strs strings to join
* @param delim the delimitor
* @return string joined by delim
*/
BOTAN_DLL std::string string_join(const std::vector<std::string>& strs,
                                  char delim);

/**
* Parse an ASN.1 OID
* @param oid the OID in string form
* @return OID components
*/
BOTAN_DLL std::vector<u32bit> parse_asn1_oid(const std::string& oid);

/**
* Compare two names using the X.509 comparison algorithm
* @param name1 the first name
* @param name2 the second name
* @return true if name1 is the same as name2 by the X.509 comparison rules
*/
BOTAN_DLL bool x500_name_cmp(const std::string& name1,
                             const std::string& name2);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_DLL u32bit to_u32bit(const std::string& str);

/**
* Convert a time specification to a number
* @param timespec the time specification
* @return number of seconds represented by timespec
*/
BOTAN_DLL u32bit timespec_to_u32bit(const std::string& timespec);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
BOTAN_DLL u32bit string_to_ipv4(const std::string& ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
BOTAN_DLL std::string ipv4_to_string(u32bit ip_addr);

std::map<std::string, std::string> BOTAN_DLL read_cfg(std::istream& is);

std::string BOTAN_DLL clean_ws(const std::string& s);


}


namespace Botan {

typedef std::runtime_error Exception;
typedef std::invalid_argument Invalid_Argument;

/**
* Invalid_State Exception
*/
struct BOTAN_DLL Invalid_State : public Exception
   {
   Invalid_State(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Lookup_Error Exception
*/
struct BOTAN_DLL Lookup_Error : public Exception
   {
   Lookup_Error(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Internal_Error Exception
*/
struct BOTAN_DLL Internal_Error : public Exception
   {
   Internal_Error(const std::string& err) :
      Exception("Internal error: " + err)
      {}
   };

/**
* Invalid_Key_Length Exception
*/
struct BOTAN_DLL Invalid_Key_Length : public Invalid_Argument
   {
   Invalid_Key_Length(const std::string& name, size_t length) :
      Invalid_Argument(name + " cannot accept a key of length " +
                       std::to_string(length))
      {}
   };

/**
* Invalid_IV_Length Exception
*/
struct BOTAN_DLL Invalid_IV_Length : public Invalid_Argument
   {
   Invalid_IV_Length(const std::string& mode, size_t bad_len) :
      Invalid_Argument("IV length " + std::to_string(bad_len) +
                       " is invalid for " + mode)
      {}
   };

/**
* PRNG_Unseeded Exception
*/
struct BOTAN_DLL PRNG_Unseeded : public Invalid_State
   {
   PRNG_Unseeded(const std::string& algo) :
      Invalid_State("PRNG not seeded: " + algo)
      {}
   };

/**
* Policy_Violation Exception
*/
struct BOTAN_DLL Policy_Violation : public Invalid_State
   {
   Policy_Violation(const std::string& err) :
      Invalid_State("Policy violation: " + err)
      {}
   };

/**
* Algorithm_Not_Found Exception
*/
struct BOTAN_DLL Algorithm_Not_Found : public Lookup_Error
   {
   Algorithm_Not_Found(const std::string& name) :
      Lookup_Error("Could not find any algorithm named \"" + name + "\"")
      {}
   };

/**
* Invalid_Algorithm_Name Exception
*/
struct BOTAN_DLL Invalid_Algorithm_Name : public Invalid_Argument
   {
   Invalid_Algorithm_Name(const std::string& name):
      Invalid_Argument("Invalid algorithm name: " + name)
      {}
   };

/**
* Encoding_Error Exception
*/
struct BOTAN_DLL Encoding_Error : public Invalid_Argument
   {
   Encoding_Error(const std::string& name) :
      Invalid_Argument("Encoding error: " + name) {}
   };

/**
* Decoding_Error Exception
*/
struct BOTAN_DLL Decoding_Error : public Invalid_Argument
   {
   Decoding_Error(const std::string& name) :
      Invalid_Argument("Decoding error: " + name) {}
   };

/**
* Integrity_Failure Exception
*/
struct BOTAN_DLL Integrity_Failure : public Exception
   {
   Integrity_Failure(const std::string& msg) :
      Exception("Integrity failure: " + msg) {}
   };

/**
* Invalid_OID Exception
*/
struct BOTAN_DLL Invalid_OID : public Decoding_Error
   {
   Invalid_OID(const std::string& oid) :
      Decoding_Error("Invalid ASN.1 OID: " + oid) {}
   };

/**
* Stream_IO_Error Exception
*/
struct BOTAN_DLL Stream_IO_Error : public Exception
   {
   Stream_IO_Error(const std::string& err) :
      Exception("I/O error: " + err)
      {}
   };

/**
* Self Test Failure Exception
*/
struct BOTAN_DLL Self_Test_Failure : public Internal_Error
   {
   Self_Test_Failure(const std::string& err) :
      Internal_Error("Self test failed: " + err)
      {}
   };

/**
* Memory Allocation Exception
*/
struct BOTAN_DLL Memory_Exhaustion : public std::bad_alloc
   {
   const char* what() const BOTAN_NOEXCEPT
      { return "Ran out of memory, allocation failed"; }
   };

}


namespace Botan {

/**
* Octet String
*/
class BOTAN_DLL OctetString
   {
   public:
      /**
      * @return size of this octet string in bytes
      */
      size_t length() const { return bits.size(); }
      size_t size() const { return bits.size(); }

      /**
      * @return this object as a secure_vector<byte>
      */
      secure_vector<byte> bits_of() const { return bits; }

      /**
      * @return start of this string
      */
      const byte* begin() const { return &bits[0]; }

      /**
      * @return end of this string
      */
      const byte* end() const   { return begin() + bits.size(); }

      /**
      * @return this encoded as hex
      */
      std::string as_string() const;

      /**
      * XOR the contents of another octet string into this one
      * @param other octet string
      * @return reference to this
      */
      OctetString& operator^=(const OctetString& other);

      /**
      * Force to have odd parity
      */
      void set_odd_parity();

      /**
      * Create a new OctetString
      * @param str is a hex encoded string
      */
      OctetString(const std::string& str = "");

      /**
      * Create a new random OctetString
      * @param rng is a random number generator
      * @param len is the desired length in bytes
      */
      OctetString(class RandomNumberGenerator& rng, size_t len);

      /**
      * Create a new OctetString
      * @param in is an array
      * @param len is the length of in in bytes
      */
      OctetString(const byte in[], size_t len);

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const secure_vector<byte>& in) : bits(in) {}

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const std::vector<byte>& in) : bits(in.begin(), in.end()) {}
   private:
      secure_vector<byte> bits;
   };

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is equal to y
*/
BOTAN_DLL bool operator==(const OctetString& x,
                          const OctetString& y);

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is not equal to y
*/
BOTAN_DLL bool operator!=(const OctetString& x,
                          const OctetString& y);

/**
* Concatenate two strings
* @param x an octet string
* @param y an octet string
* @return x concatenated with y
*/
BOTAN_DLL OctetString operator+(const OctetString& x,
                                const OctetString& y);

/**
* XOR two strings
* @param x an octet string
* @param y an octet string
* @return x XORed with y
*/
BOTAN_DLL OctetString operator^(const OctetString& x,
                                const OctetString& y);


/**
* Alternate name for octet string showing intent to use as a key
*/
typedef OctetString SymmetricKey;

/**
* Alternate name for octet string showing intent to use as an IV
*/
typedef OctetString InitializationVector;

}


namespace Botan {

/**
* Interface for general transformations on data
*/
class BOTAN_DLL Transform
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      secure_vector<byte> start(const std::vector<byte, Alloc>& nonce)
         {
         return start(&nonce[0], nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      BOTAN_DEPRECATED("Use Transform::start")
      secure_vector<byte> start_vec(const std::vector<byte, Alloc>& nonce)
         {
         return start(&nonce[0], nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      * @param nonce_len length of nonce
      */
      secure_vector<byte> start(const byte nonce[], size_t nonce_len)
         {
         return start_raw(nonce, nonce_len);
         }

      /**
      * Begin processing a message.
      */
      secure_vector<byte> start()
         {
         return start_raw(nullptr, 0);
         }

      virtual secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) = 0;

      /**
      * Process some data. Input must be in size update_granularity() byte blocks.
      * @param blocks in/out paramter which will possibly be resized
      * @param offset an offset into blocks to begin processing
      */
      virtual void update(secure_vector<byte>& blocks, size_t offset = 0) = 0;

      /**
      * Complete processing of a message.
      *
      * @param final_block in/out parameter which must be at least
      *        minimum_final_size() bytes, and will be set to any final output
      * @param offset an offset into final_block to begin processing
      */
      virtual void finish(secure_vector<byte>& final_block, size_t offset = 0) = 0;

      /**
      * Returns the size of the output if this transform is used to process a
      * message with input_length bytes. Will throw if unable to give a precise
      * answer.
      */
      virtual size_t output_length(size_t input_length) const = 0;

      /**
      * @return size of required blocks to update
      */
      virtual size_t update_granularity() const = 0;

      /**
      * @return required minimium size to finalize() - may be any
      *         length larger than this.
      */
      virtual size_t minimum_final_size() const = 0;

      /**
      * Return the default size for a nonce
      */
      virtual size_t default_nonce_length() const = 0;

      /**
      * Return true iff nonce_len is a valid length for the nonce
      */
      virtual bool valid_nonce_length(size_t nonce_len) const = 0;

      /**
      * Return some short name describing the provider of this tranformation.
      * Useful in cases where multiple implementations are available (eg,
      * different implementations of AES). Default "core" is used for the
      * 'standard' implementation included in the library.
      */
      virtual std::string provider() const { return "core"; }

      virtual std::string name() const = 0;

      virtual void clear() = 0;

      virtual ~Transform() {}
   };

class BOTAN_DLL Keyed_Transform : public Transform
   {
   public:
      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(&key[0], key.size());
         }

      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

   private:
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

typedef Transform Transformation;

BOTAN_DLL Transform* get_transform(const std::string& specstr,
                                   const std::string& provider = "",
                                   const std::string& dirstr = "");

}


namespace Botan {

/**
* This class represents a symmetric algorithm object.
*/
class BOTAN_DLL SymmetricAlgorithm
   {
   public:
      virtual ~SymmetricAlgorithm() {}

      virtual void clear() = 0;

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * @return minimum allowed key length
      */
      size_t maximum_keylength() const
         {
         return key_spec().maximum_keylength();
         }

      /**
      * @return maxmium allowed key length
      */
      size_t minimum_keylength() const
         {
         return key_spec().minimum_keylength();
         }

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * Set the symmetric key of this object.
      * @param key the SymmetricKey to be set.
      */
      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(&key[0], key.size());
         }

      /**
      * Set the symmetric key of this object.
      * @param key the to be set as a byte array.
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

      virtual std::string name() const = 0;

   private:
      /**
      * Run the key schedule
      * @param key the key
      * @param length of key
      */
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

}


namespace Botan {

/**
* Base class for all stream ciphers
*/
class BOTAN_DLL StreamCipher : public SymmetricAlgorithm
   {
   public:
      /**
      * Encrypt or decrypt a message
      * @param in the plaintext
      * @param out the byte array to hold the output, i.e. the ciphertext
      * @param len the length of both in and out in bytes
      */
      virtual void cipher(const byte in[], byte out[], size_t len) = 0;

      /**
      * Encrypt or decrypt a message
      * @param buf the plaintext / ciphertext
      * @param len the length of buf in bytes
      */
      void cipher1(byte buf[], size_t len)
         { cipher(buf, buf, len); }

      template<typename Alloc>
         void encipher(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      template<typename Alloc>
         void encrypt(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      template<typename Alloc>
         void decrypt(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      /**
      * Resync the cipher using the IV
      * @param iv the initialization vector
      * @param iv_len the length of the IV in bytes
      */
      virtual void set_iv(const byte[], size_t iv_len)
         {
         if(iv_len)
            throw Invalid_IV_Length(name(), iv_len);
         }

      /**
      * @param iv_len the length of the IV in bytes
      * @return if the length is valid for this algorithm
      */
      virtual bool valid_iv_length(size_t iv_len) const { return (iv_len == 0); }

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual StreamCipher* clone() const = 0;

      typedef SCAN_Name Spec;
   };

}


namespace Botan {

/**
* Interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode : public Keyed_Transform
   {
   public:
      /**
      * Returns true iff this mode provides authentication as well as
      * confidentiality.
      */
      virtual bool authenticated() const { return false; }

      /**
      * Return the size of the authentication tag used (in bytes)
      */
      virtual size_t tag_size() const { return 0; }
   };

/**
* The two possible directions for cipher filters, determining whether they
* actually perform encryption or decryption.
*/
enum Cipher_Dir { ENCRYPTION, DECRYPTION };

BOTAN_DLL Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction);

}


namespace Botan {

/**
* Interface for AEAD (Authenticated Encryption with Associated Data)
* modes. These modes provide both encryption and message
* authentication, and can authenticate additional per-message data
* which is not included in the ciphertext (for instance a sequence
* number).
*/
class BOTAN_DLL AEAD_Mode : public Cipher_Mode
   {
   public:
      bool authenticated() const override { return true; }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * Unless reset by another call, the associated data is kept
      * between messages. Thus, if the AD does not change, calling
      * once (after set_key) is the optimum.
      *
      * @param ad the associated data
      * @param ad_len length of add in bytes
      */
      virtual void set_associated_data(const byte ad[], size_t ad_len) = 0;

      template<typename Alloc>
      void set_associated_data_vec(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(&ad[0], ad.size());
         }

      template<typename Alloc>
      void set_ad(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(&ad[0], ad.size());
         }

      /**
      * Default AEAD nonce size (a commonly supported value among AEAD
      * modes, and large enough that random collisions are unlikely).
      */
      size_t default_nonce_length() const override { return 12; }
   };

/**
* Get an AEAD mode by name (eg "AES-128/GCM" or "Serpent/EAX")
*/
BOTAN_DLL AEAD_Mode* get_aead(const std::string& name, Cipher_Dir direction);

}


namespace Botan {

/**
* This class represents a block cipher object.
*/
class BOTAN_DLL BlockCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * @return block size of this algorithm
      */
      virtual size_t block_size() const = 0;

      /**
      * @return native parallelism of this cipher in blocks
      */
      virtual size_t parallelism() const { return 1; }

      /**
      * @return prefererred parallelism of this cipher in bytes
      */
      size_t parallel_bytes() const
         {
         return parallelism() * block_size() * BOTAN_BLOCK_CIPHER_PAR_MULT;
         }

      /**
      * Encrypt a block.
      * @param in The plaintext block to be encrypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the encrypted block.
      * Must be of length block_size().
      */
      void encrypt(const byte in[], byte out[]) const
         { encrypt_n(in, out, 1); }

      /**
      * Decrypt a block.
      * @param in The ciphertext block to be decypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the decrypted block.
      * Must be of length block_size().
      */
      void decrypt(const byte in[], byte out[]) const
         { decrypt_n(in, out, 1); }

      /**
      * Encrypt a block.
      * @param block the plaintext block to be encrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void encrypt(byte block[]) const { encrypt_n(block, block, 1); }

      /**
      * Decrypt a block.
      * @param block the ciphertext block to be decrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void decrypt(byte block[]) const { decrypt_n(block, block, 1); }

      /**
      * Encrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void encrypt(std::vector<byte, Alloc>& block) const
         {
         return encrypt_n(&block[0], &block[0], block.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void decrypt(std::vector<byte, Alloc>& block) const
         {
         return decrypt_n(&block[0], &block[0], block.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void encrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return encrypt_n(&in[0], &out[0], in.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void decrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return decrypt_n(&in[0], &out[0], in.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void encrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void decrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual BlockCipher* clone() const = 0;
   };

/**
* Represents a block cipher with a single fixed block size
*/
template<size_t BS, size_t KMIN, size_t KMAX = 0, size_t KMOD = 1>
class Block_Cipher_Fixed_Params : public BlockCipher
   {
   public:
      enum { BLOCK_SIZE = BS };
      size_t block_size() const { return BS; }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(KMIN, KMAX, KMOD);
         }
   };

}


namespace Botan {

/**
* AES-128
*/
class BOTAN_DLL AES_128 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();

      std::string name() const { return "AES-128"; }
      BlockCipher* clone() const { return new AES_128; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-192
*/
class BOTAN_DLL AES_192 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();

      std::string name() const { return "AES-192"; }
      BlockCipher* clone() const { return new AES_192; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-256
*/
class BOTAN_DLL AES_256 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();

      std::string name() const { return "AES-256"; }
      BlockCipher* clone() const { return new AES_256; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

}


namespace Botan {

/**
* AES-128 using AES-NI
*/
class BOTAN_DLL AES_128_NI : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-128"; }
      BlockCipher* clone() const { return new AES_128_NI; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-192 using AES-NI
*/
class BOTAN_DLL AES_192_NI : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-192"; }
      BlockCipher* clone() const { return new AES_192_NI; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-256 using AES-NI
*/
class BOTAN_DLL AES_256_NI : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-256"; }
      BlockCipher* clone() const { return new AES_256_NI; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

}


namespace Botan {

/**
* AES-128 using SSSE3
*/
class BOTAN_DLL AES_128_SSSE3 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-128"; }
      BlockCipher* clone() const { return new AES_128_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-192 using SSSE3
*/
class BOTAN_DLL AES_192_SSSE3 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-192"; }
      BlockCipher* clone() const { return new AES_192_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-256 using SSSE3
*/
class BOTAN_DLL AES_256_SSSE3 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-256"; }
      BlockCipher* clone() const { return new AES_256_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

}


namespace Botan {

/**
* ASN.1 Type and Class Tags
*/
enum ASN1_Tag {
   UNIVERSAL        = 0x00,
   APPLICATION      = 0x40,
   CONTEXT_SPECIFIC = 0x80,

   CONSTRUCTED      = 0x20,

   PRIVATE          = CONSTRUCTED | CONTEXT_SPECIFIC,

   EOC              = 0x00,
   BOOLEAN          = 0x01,
   INTEGER          = 0x02,
   BIT_STRING       = 0x03,
   OCTET_STRING     = 0x04,
   NULL_TAG         = 0x05,
   OBJECT_ID        = 0x06,
   ENUMERATED       = 0x0A,
   SEQUENCE         = 0x10,
   SET              = 0x11,

   UTF8_STRING      = 0x0C,
   NUMERIC_STRING   = 0x12,
   PRINTABLE_STRING = 0x13,
   T61_STRING       = 0x14,
   IA5_STRING       = 0x16,
   VISIBLE_STRING   = 0x1A,
   BMP_STRING       = 0x1E,

   UTC_TIME         = 0x17,
   GENERALIZED_TIME = 0x18,

   NO_OBJECT        = 0xFF00,
   DIRECTORY_STRING = 0xFF01
};

/**
* Basic ASN.1 Object Interface
*/
class BOTAN_DLL ASN1_Object
   {
   public:
      /**
      * Encode whatever this object is into to
      * @param to the DER_Encoder that will be written to
      */
      virtual void encode_into(class DER_Encoder& to) const = 0;

      /**
      * Decode whatever this object is from from
      * @param from the BER_Decoder that will be read from
      */
      virtual void decode_from(class BER_Decoder& from) = 0;

      virtual ~ASN1_Object() {}
   };

/**
* BER Encoded Object
*/
class BOTAN_DLL BER_Object
   {
   public:
      void assert_is_a(ASN1_Tag, ASN1_Tag);

      ASN1_Tag type_tag, class_tag;
      secure_vector<byte> value;
   };

/*
* ASN.1 Utility Functions
*/
class DataSource;

namespace ASN1 {

std::vector<byte> put_in_sequence(const std::vector<byte>& val);
std::string to_string(const BER_Object& obj);

/**
* Heuristics tests; is this object possibly BER?
* @param src a data source that will be peeked at but not modified
*/
bool maybe_BER(DataSource& src);

}

/**
* General BER Decoding Error Exception
*/
struct BOTAN_DLL BER_Decoding_Error : public Decoding_Error
   {
   BER_Decoding_Error(const std::string&);
   };

/**
* Exception For Incorrect BER Taggings
*/
struct BOTAN_DLL BER_Bad_Tag : public BER_Decoding_Error
   {
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag);
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag1, ASN1_Tag tag2);
   };

}


namespace Botan {

/**
* This class represents ASN.1 object identifiers.
*/
class BOTAN_DLL OID : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      /**
      * Find out whether this OID is empty
      * @return true is no OID value is set
      */
      bool empty() const { return id.size() == 0; }

      /**
      * Get this OID as list (vector) of its components.
      * @return vector representing this OID
      */
      const std::vector<u32bit>& get_id() const { return id; }

      /**
      * Get this OID as a string
      * @return string representing this OID
      */
      std::string as_string() const;

      /**
      * Compare two OIDs.
      * @return true if they are equal, false otherwise
      */
      bool operator==(const OID&) const;

      /**
      * Reset this instance to an empty OID.
      */
      void clear();

      /**
      * Add a component to this OID.
      * @param new_comp the new component to add to the end of this OID
      * @return reference to *this
      */
      OID& operator+=(u32bit new_comp);

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      */
      OID(const std::string& str = "");
   private:
      std::vector<u32bit> id;
   };

/**
* Append another component onto the OID.
* @param oid the OID to add the new component to
* @param new_comp the new component to add
*/
OID BOTAN_DLL operator+(const OID& oid, u32bit new_comp);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is not equal to b
*/
bool BOTAN_DLL operator!=(const OID& a, const OID& b);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is lexicographically smaller than b
*/
bool BOTAN_DLL operator<(const OID& a, const OID& b);

}


namespace Botan {

/**
* Algorithm Identifier
*/
class BOTAN_DLL AlgorithmIdentifier : public ASN1_Object
   {
   public:
      enum Encoding_Option { USE_NULL_PARAM };

      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      AlgorithmIdentifier() {}
      AlgorithmIdentifier(const OID&, Encoding_Option);
      AlgorithmIdentifier(const std::string&, Encoding_Option);

      AlgorithmIdentifier(const OID&, const std::vector<byte>&);
      AlgorithmIdentifier(const std::string&, const std::vector<byte>&);

      OID oid;
      std::vector<byte> parameters;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);
bool BOTAN_DLL operator!=(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);

}


namespace Botan {

/**
* Simple String
*/
class BOTAN_DLL ASN1_String : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      std::string value() const;
      std::string iso_8859() const;

      ASN1_Tag tagging() const;

      ASN1_String(const std::string& = "");
      ASN1_String(const std::string&, ASN1_Tag);
   private:
      std::string iso_8859_str;
      ASN1_Tag tag;
   };

}


namespace Botan {

/**
* Alternative Name
*/
class BOTAN_DLL AlternativeName : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      std::multimap<std::string, std::string> get_attributes() const;

      void add_othername(const OID&, const std::string&, ASN1_Tag);
      std::multimap<OID, ASN1_String> get_othernames() const;

      bool has_items() const;

      AlternativeName(const std::string& = "", const std::string& = "",
                      const std::string& = "", const std::string& = "");
   private:
      std::multimap<std::string, std::string> alt_info;
      std::multimap<OID, ASN1_String> othernames;
   };

}


namespace Botan {

/**
* Attribute
*/
class BOTAN_DLL Attribute : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder& to) const;
      void decode_from(class BER_Decoder& from);

      OID oid;
      std::vector<byte> parameters;

      Attribute() {}
      Attribute(const OID&, const std::vector<byte>&);
      Attribute(const std::string&, const std::vector<byte>&);
   };

}


namespace Botan {

/**
* X.509 Time
*/
class BOTAN_DLL X509_Time : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      std::string as_string() const;
      std::string readable_string() const;
      bool time_is_set() const;

      std::string to_string() const { return readable_string(); }

      s32bit cmp(const X509_Time&) const;

      void set_to(const std::string&);
      void set_to(const std::string&, ASN1_Tag);

      X509_Time(const std::chrono::system_clock::time_point& time);
      X509_Time(const std::string& = "");
      X509_Time(const std::string&, ASN1_Tag);
   private:
      bool passes_sanity_check() const;
      u32bit year, month, day, hour, minute, second;
      ASN1_Tag tag;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator!=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>(const X509_Time&, const X509_Time&);

}


namespace Botan {

/**
* Class used to accumulate the poll results of EntropySources
*/
class BOTAN_DLL Entropy_Accumulator
   {
   public:
      /**
      * Initialize an Entropy_Accumulator
      * @param goal is how many bits we would like to collect
      */
      Entropy_Accumulator(std::function<bool (const byte[], size_t, double)> accum) :
         m_accum_fn(accum), m_done(false) {}

      virtual ~Entropy_Accumulator() {}

      /**
      * Get a cached I/O buffer (purely for minimizing allocation
      * overhead to polls)
      *
      * @param size requested size for the I/O buffer
      * @return cached I/O buffer for repeated polls
      */
      secure_vector<byte>& get_io_buffer(size_t size)
         {
         m_io_buffer.clear();
         m_io_buffer.resize(size);
         return m_io_buffer;
         }

      /**
      * @return if our polling goal has been achieved
      */
      bool polling_goal_achieved() const { return m_done; }

      /**
      * Add entropy to the accumulator
      * @param bytes the input bytes
      * @param length specifies how many bytes the input is
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      void add(const void* bytes, size_t length, double entropy_bits_per_byte)
         {
         m_done = m_accum_fn(reinterpret_cast<const byte*>(bytes),
                             length, entropy_bits_per_byte * length);
         }

      /**
      * Add entropy to the accumulator
      * @param v is some value
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      template<typename T>
      void add(const T& v, double entropy_bits_per_byte)
         {
         add(&v, sizeof(T), entropy_bits_per_byte);
         }
   private:
      std::function<bool (const byte[], size_t, double)> m_accum_fn;
      bool m_done;
      secure_vector<byte> m_io_buffer;
   };

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL EntropySource
   {
   public:
      static void poll_available_sources(class Entropy_Accumulator& accum);

      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param accum is an accumulator object that will be given entropy
      */
      virtual void poll(Entropy_Accumulator& accum) = 0;

      virtual ~EntropySource() {}
   };

}


namespace Botan {

/**
* This class represents a random number (RNG) generator object.
*/
class BOTAN_DLL RandomNumberGenerator
   {
   public:
      /**
      * Create a seeded and active RNG object for general application use
      * Added in 1.8.0
      */
      static RandomNumberGenerator* make_rng();

      /**
      * Randomize a byte array.
      * @param output the byte array to hold the random output.
      * @param length the length of the byte array output.
      */
      virtual void randomize(byte output[], size_t length) = 0;

      /**
      * Return a random vector
      * @param bytes number of bytes in the result
      * @return randomized vector of length bytes
      */
      virtual secure_vector<byte> random_vec(size_t bytes)
         {
         secure_vector<byte> output(bytes);
         randomize(&output[0], output.size());
         return output;
         }

      /**
      * Return a random byte
      * @return random byte
      */
      byte next_byte()
         {
         byte out;
         this->randomize(&out, 1);
         return out;
         }

      /**
      * Check whether this RNG is seeded.
      * @return true if this RNG was already seeded, false otherwise.
      */
      virtual bool is_seeded() const = 0;

      /**
      * Clear all internally held values of this RNG.
      */
      virtual void clear() = 0;

      /**
      * Return the name of this object
      */
      virtual std::string name() const = 0;

      /**
      * Seed this RNG using the entropy sources it contains.
      * @param bits_to_collect is the number of bits of entropy to
               attempt to gather from the entropy sources
      */
      virtual void reseed(size_t bits_to_collect) = 0;

      /**
      * Add entropy to this RNG.
      * @param in a byte array containg the entropy to be added
      * @param length the length of the byte array in
      */
      virtual void add_entropy(const byte in[], size_t length) = 0;

      /*
      * Never copy a RNG, create a new one
      */
      RandomNumberGenerator(const RandomNumberGenerator& rng) = delete;
      RandomNumberGenerator& operator=(const RandomNumberGenerator& rng) = delete;

      RandomNumberGenerator() {}
      virtual ~RandomNumberGenerator() {}
   };

/**
* Null/stub RNG - fails if you try to use it for anything
*/
class BOTAN_DLL Null_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte[], size_t) override { throw PRNG_Unseeded("Null_RNG"); }

      void clear() override {}

      std::string name() const override { return "Null_RNG"; }

      void reseed(size_t) override {}
      bool is_seeded() const override { return false; }
      void add_entropy(const byte[], size_t) override {}
   };

/**
* Wraps access to a RNG in a mutex
*/
class BOTAN_DLL Serialized_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->randomize(out, len);
         }

      bool is_seeded() const
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->is_seeded();
         }

      void clear()
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->clear();
         }

      std::string name() const
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->name();
         }

      void reseed(size_t poll_bits)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->reseed(poll_bits);
         }

      void add_entropy(const byte in[], size_t len)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->add_entropy(in, len);
         }

      Serialized_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      mutable std::mutex m_mutex;
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

class BOTAN_DLL AutoSeeded_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len)
         { m_rng->randomize(out, len); }

      bool is_seeded() const { return m_rng->is_seeded(); }

      void clear() { m_rng->clear(); }

      std::string name() const { return m_rng->name(); }

      void reseed(size_t poll_bits = 256) { m_rng->reseed(poll_bits); }

      void add_entropy(const byte in[], size_t len)
         { m_rng->add_entropy(in, len); }

      AutoSeeded_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

/**
* This class represents general abstract filter objects.
*/
class BOTAN_DLL Filter
   {
   public:
      /**
      * @return descriptive name for this filter
      */
      virtual std::string name() const = 0;

      /**
      * Write a portion of a message to this filter.
      * @param input the input as a byte array
      * @param length the length of the byte array input
      */
      virtual void write(const byte input[], size_t length) = 0;

      /**
      * Start a new message. Must be closed by end_msg() before another
      * message can be started.
      */
      virtual void start_msg() {}

      /**
      * Notify that the current message is finished; flush buffers and
      * do end-of-message processing (if any).
      */
      virtual void end_msg() {}

      /**
      * Check whether this filter is an attachable filter.
      * @return true if this filter is attachable, false otherwise
      */
      virtual bool attachable() { return true; }

      virtual ~Filter() {}
   protected:
      /**
      * @param in some input for the filter
      * @param length the length of in
      */
      virtual void send(const byte in[], size_t length);

      /**
      * @param in some input for the filter
      */
      void send(byte in) { send(&in, 1); }

      /**
      * @param in some input for the filter
      */
      void send(const secure_vector<byte>& in) { send(&in[0], in.size()); }

      /**
      * @param in some input for the filter
      */
      void send(const std::vector<byte>& in) { send(&in[0], in.size()); }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const secure_vector<byte>& in, size_t length)
         {
         send(&in[0], length);
         }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const std::vector<byte>& in, size_t length)
         {
         send(&in[0], length);
         }

      Filter();

      Filter(const Filter&) = delete;

      Filter& operator=(const Filter&) = delete;

   private:
      /**
      * Start a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void new_msg();

      /**
      * End a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void finish_msg();

      friend class Pipe;
      friend class Fanout_Filter;

      size_t total_ports() const;
      size_t current_port() const { return port_num; }

      /**
      * Set the active port
      * @param new_port the new value
      */
      void set_port(size_t new_port);

      size_t owns() const { return filter_owns; }

      /**
      * Attach another filter to this one
      * @param f filter to attach
      */
      void attach(Filter* f);

      /**
      * @param filters the filters to set
      * @param count number of items in filters
      */
      void set_next(Filter* filters[], size_t count);
      Filter* get_next() const;

      secure_vector<byte> write_queue;
      std::vector<Filter*> next;
      size_t port_num, filter_owns;

      // true if filter belongs to a pipe --> prohibit filter sharing!
      bool owned;
   };

/**
* This is the abstract Fanout_Filter base class.
**/
class BOTAN_DLL Fanout_Filter : public Filter
   {
   protected:
      /**
      * Increment the number of filters past us that we own
      */
      void incr_owns() { ++filter_owns; }

      void set_port(size_t n) { Filter::set_port(n); }

      void set_next(Filter* f[], size_t n) { Filter::set_next(f, n); }

      void attach(Filter* f) { Filter::attach(f); }

   private:
      friend class Threaded_Fork;
      using Filter::write_queue;
      using Filter::total_ports;
      using Filter::next;
   };

/**
* The type of checking to be performed by decoders:
* NONE - no checks, IGNORE_WS - perform checks, but ignore
* whitespaces, FULL_CHECK - perform checks, also complain
* about white spaces.
*/
enum Decoder_Checking { NONE, IGNORE_WS, FULL_CHECK };

}


namespace Botan {

/**
* This class represents a Base64 encoder.
*/
class BOTAN_DLL Base64_Encoder : public Filter
   {
   public:
      std::string name() const { return "Base64_Encoder"; }

      /**
      * Input a part of a message to the encoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length);

      /**
      * Inform the Encoder that the current message shall be closed.
      */
      void end_msg();

      /**
      * Create a base64 encoder.
      * @param breaks whether to use line breaks in the output
      * @param length the length of the lines of the output
      * @param t_n whether to use a trailing newline
      */
      Base64_Encoder(bool breaks = false, size_t length = 72,
                     bool t_n = false);
   private:
      void encode_and_send(const byte input[], size_t length,
                           bool final_inputs = false);
      void do_output(const byte output[], size_t length);

      const size_t line_length;
      const bool trailing_newline;
      std::vector<byte> in, out;
      size_t position, out_position;
   };

/**
* This object represents a Base64 decoder.
*/
class BOTAN_DLL Base64_Decoder : public Filter
   {
   public:
      std::string name() const { return "Base64_Decoder"; }

      /**
      * Input a part of a message to the decoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length);

      /**
      * Finish up the current message
      */
      void end_msg();

      /**
      * Create a base64 decoder.
      * @param checking the type of checking that shall be performed by
      * the decoder
      */
      Base64_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking checking;
      std::vector<byte> in, out;
      size_t position;
   };

}


namespace Botan {

/**
* Perform base64 encoding
* @param output an array of at least input_length*4/3 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_encode(char output[],
                               const byte input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs);

/**
* Perform base64 encoding
* @param input some input
* @param input_length length of input in bytes
* @return base64adecimal representation of input
*/
std::string BOTAN_DLL base64_encode(const byte input[],
                                    size_t input_length);

/**
* Perform base64 encoding
* @param input some input
* @return base64adecimal representation of input
*/
template<typename Alloc>
std::string base64_encode(const std::vector<byte, Alloc>& input)
   {
   return base64_encode(&input[0], input.size());
   }

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length/3*4 bytes
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const std::string& input,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const char input[],
                                           size_t input_length,
                                           bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const std::string& input,
                                           bool ignore_ws = true);

}


namespace Botan {

/**
* BitBucket is a filter which simply discards all inputs
*/
struct BOTAN_DLL BitBucket : public Filter
   {
   void write(const byte[], size_t) {}

   std::string name() const { return "BitBucket"; }
   };

/**
* This class represents Filter chains. A Filter chain is an ordered
* concatenation of Filters, the input to a Chain sequentially passes
* through all the Filters contained in the Chain.
*/

class BOTAN_DLL Chain : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) { send(input, length); }

      std::string name() const;

      /**
      * Construct a chain of up to four filters. The filters are set
      * up in the same order as the arguments.
      */
      Chain(Filter* = nullptr, Filter* = nullptr,
            Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a chain from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Chain(Filter* filter_arr[], size_t length);
   };

/**
* This class represents a fork filter, whose purpose is to fork the
* flow of data. It causes an input message to result in n messages at
* the end of the filter, where n is the number of forks.
*/
class BOTAN_DLL Fork : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) { send(input, length); }
      void set_port(size_t n) { Fanout_Filter::set_port(n); }

      std::string name() const;

      /**
      * Construct a Fork filter with up to four forks.
      */
      Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Fork(Filter* filter_arr[], size_t length);
   };

/**
* This class is a threaded version of the Fork filter. While this uses
* threads, the class itself is NOT thread-safe. This is meant as a drop-
* in replacement for Fork where performance gains are possible.
*/
class BOTAN_DLL Threaded_Fork : public Fork
   {
   public:
      std::string name() const;

      /**
      * Construct a Threaded_Fork filter with up to four forks.
      */
      Threaded_Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Threaded_Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Threaded_Fork(Filter* filter_arr[], size_t length);

      ~Threaded_Fork();

   protected:
      void set_next(Filter* f[], size_t n);
      void send(const byte in[], size_t length);

   private:
      void thread_delegate_work(const byte input[], size_t length);
      void thread_entry(Filter* filter);

      std::vector<std::shared_ptr<std::thread>> m_threads;
      std::unique_ptr<struct Threaded_Fork_Data> m_thread_data;
   };

}


namespace Botan {

/**
* Create a password hash using Bcrypt
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
*
* @see http://www.usenix.org/events/usenix99/provos/provos_html/
*/
std::string BOTAN_DLL generate_bcrypt(const std::string& password,
                                      RandomNumberGenerator& rng,
                                      u16bit work_factor = 10);

/**
* Check a previously created password hash
* @param password the password to check against
* @param hash the stored hash to check against
*/
bool BOTAN_DLL check_bcrypt(const std::string& password,
                            const std::string& hash);

}


namespace Botan {

/**
* Algorithm benchmark
* @param name the name of the algorithm to test (cipher, hash, or MAC)
* @param af the algorithm factory used to create objects
* @param rng the rng to use to generate random inputs
* @param milliseconds total time for the benchmark to run
* @param buf_size size of buffer to benchmark against, in KiB
* @return results a map from provider to speed in mebibytes per second
*/
std::map<std::string, double>
BOTAN_DLL algorithm_benchmark(const std::string& name,
                              RandomNumberGenerator& rng,
                              std::chrono::milliseconds milliseconds,
                              size_t buf_size);

}


namespace Botan {

/**
* This class represents an abstract data source object.
*/
class BOTAN_DLL DataSource
   {
   public:
      /**
      * Read from the source. Moves the internal offset so that every
      * call to read will return a new portion of the source.
      *
      * @param out the byte array to write the result to
      * @param length the length of the byte array out
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t read(byte out[], size_t length) = 0;

      /**
      * Read from the source but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the source starting at the same position.
      *
      * @param out the byte array to write the output to
      * @param length the length of the byte array out
      * @param peek_offset the offset into the stream to read at
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t peek(byte out[], size_t length,
                          size_t peek_offset) const = 0;

      /**
      * Test whether the source still has data that can be read.
      * @return true if there is still data to read, false otherwise
      */
      virtual bool end_of_data() const = 0;
      /**
      * return the id of this data source
      * @return std::string representing the id of this data source
      */
      virtual std::string id() const { return ""; }

      /**
      * Read one byte.
      * @param out the byte to read to
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t read_byte(byte& out);

      /**
      * Peek at one byte.
      * @param out an output byte
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t peek_byte(byte& out) const;

      /**
      * Discard the next N bytes of the data
      * @param N the number of bytes to discard
      * @return number of bytes actually discarded
      */
      size_t discard_next(size_t N);

      /**
      * @return number of bytes read so far.
      */
      virtual size_t get_bytes_read() const = 0;

      DataSource() {}
      virtual ~DataSource() {}
      DataSource& operator=(const DataSource&) = delete;
      DataSource(const DataSource&) = delete;
   };

/**
* This class represents a Memory-Based DataSource
*/
class BOTAN_DLL DataSource_Memory : public DataSource
   {
   public:
      size_t read(byte[], size_t);
      size_t peek(byte[], size_t, size_t) const;
      bool end_of_data() const;

      /**
      * Construct a memory source that reads from a string
      * @param in the string to read from
      */
      DataSource_Memory(const std::string& in);

      /**
      * Construct a memory source that reads from a byte array
      * @param in the byte array to read from
      * @param length the length of the byte array
      */
      DataSource_Memory(const byte in[], size_t length) :
         source(in, in + length), offset(0) {}

      /**
      * Construct a memory source that reads from a secure_vector
      * @param in the MemoryRegion to read from
      */
      DataSource_Memory(const secure_vector<byte>& in) :
         source(in), offset(0) {}

      /**
      * Construct a memory source that reads from a std::vector
      * @param in the MemoryRegion to read from
      */
      DataSource_Memory(const std::vector<byte>& in) :
         source(in.begin(), in.end()), offset(0) {}

      virtual size_t get_bytes_read() const { return offset; }
   private:
      secure_vector<byte> source;
      size_t offset;
   };

/**
* This class represents a Stream-Based DataSource.
*/
class BOTAN_DLL DataSource_Stream : public DataSource
   {
   public:
      size_t read(byte[], size_t);
      size_t peek(byte[], size_t, size_t) const;
      bool end_of_data() const;
      std::string id() const;

      DataSource_Stream(std::istream&,
                        const std::string& id = "<std::istream>");

      /**
      * Construct a Stream-Based DataSource from file
      * @param file the name of the file
      * @param use_binary whether to treat the file as binary or not
      */
      DataSource_Stream(const std::string& file, bool use_binary = false);

      DataSource_Stream(const DataSource_Stream&) = delete;

      DataSource_Stream& operator=(const DataSource_Stream&) = delete;

      ~DataSource_Stream();

      virtual size_t get_bytes_read() const { return total_read; }
   private:
      const std::string identifier;

      std::istream* source_p;
      std::istream& source;
      size_t total_read;
   };

}


namespace Botan {

/**
* BER Decoding Object
*/
class BOTAN_DLL BER_Decoder
   {
   public:
      BER_Object get_next_object();

      std::vector<byte> get_next_octet_string();

      void push_back(const BER_Object& obj);

      bool more_items() const;
      BER_Decoder& verify_end();
      BER_Decoder& discard_remaining();

      BER_Decoder  start_cons(ASN1_Tag type_tag, ASN1_Tag class_tag = UNIVERSAL);
      BER_Decoder& end_cons();

      BER_Decoder& get_next(BER_Object& ber);

      BER_Decoder& raw_bytes(secure_vector<byte>& v);
      BER_Decoder& raw_bytes(std::vector<byte>& v);

      BER_Decoder& decode_null();
      BER_Decoder& decode(bool& v);
      BER_Decoder& decode(size_t& v);
      BER_Decoder& decode(class BigInt& v);
      BER_Decoder& decode(std::vector<byte>& v, ASN1_Tag type_tag);
      BER_Decoder& decode(secure_vector<byte>& v, ASN1_Tag type_tag);

      BER_Decoder& decode(bool& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(size_t& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class BigInt& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(std::vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(secure_vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class ASN1_Object& obj,
                          ASN1_Tag type_tag = NO_OBJECT,
                          ASN1_Tag class_tag = NO_OBJECT);

      BER_Decoder& decode_octet_string_bigint(class BigInt& b);

      u64bit decode_constrained_integer(ASN1_Tag type_tag,
                                        ASN1_Tag class_tag,
                                        size_t T_bytes);

      template<typename T> BER_Decoder& decode_integer_type(T& out)
         {
         return decode_integer_type<T>(out, INTEGER, UNIVERSAL);
         }

      template<typename T>
         BER_Decoder& decode_integer_type(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         out = decode_constrained_integer(type_tag, class_tag, sizeof(out));
         return (*this);
         }

      template<typename T>
         BER_Decoder& decode_optional(T& out,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag,
                                      const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_optional_implicit(
            T& out,
            ASN1_Tag type_tag,
            ASN1_Tag class_tag,
            ASN1_Tag real_type,
            ASN1_Tag real_class,
            const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_list(std::vector<T>& out,
                                  ASN1_Tag type_tag = SEQUENCE,
                                  ASN1_Tag class_tag = UNIVERSAL);

      template<typename T>
         BER_Decoder& decode_and_check(const T& expected,
                                       const std::string& error_msg)
         {
         T actual;
         decode(actual);

         if(actual != expected)
            throw Decoding_Error(error_msg);

         return (*this);
         }

      /*
      * Decode an OPTIONAL string type
      */
      template<typename Alloc>
      BER_Decoder& decode_optional_string(std::vector<byte, Alloc>& out,
                                          ASN1_Tag real_type,
                                          u16bit type_no,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         BER_Object obj = get_next_object();

         ASN1_Tag type_tag = static_cast<ASN1_Tag>(type_no);

         if(obj.type_tag == type_tag && obj.class_tag == class_tag)
            {
            if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
               BER_Decoder(obj.value).decode(out, real_type).verify_end();
            else
               {
               push_back(obj);
               decode(out, real_type, type_tag, class_tag);
               }
            }
         else
            {
            out.clear();
            push_back(obj);
            }

         return (*this);
         }

      BER_Decoder& operator=(const BER_Decoder&) = delete;

      BER_Decoder(DataSource&);

      BER_Decoder(const byte[], size_t);

      BER_Decoder(const secure_vector<byte>&);

      BER_Decoder(const std::vector<byte>& vec);

      BER_Decoder(const BER_Decoder&);
      ~BER_Decoder();
   private:
      BER_Decoder* parent;
      DataSource* source;
      BER_Object pushed;
      mutable bool owns;
   };

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag,
                                          const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
         BER_Decoder(obj.value).decode(out).verify_end();
      else
         {
         push_back(obj);
         decode(out, type_tag, class_tag);
         }
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional_implicit(
   T& out,
   ASN1_Tag type_tag,
   ASN1_Tag class_tag,
   ASN1_Tag real_type,
   ASN1_Tag real_class,
   const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      obj.type_tag = real_type;
      obj.class_tag = real_class;
      push_back(obj);
      decode(out, real_type, real_class);
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }
/*
* Decode a list of homogenously typed values
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_list(std::vector<T>& vec,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag)
   {
   BER_Decoder list = start_cons(type_tag, class_tag);

   while(list.more_items())
      {
      T value;
      list.decode(value);
      vec.push_back(value);
      }

   list.end_cons();

   return (*this);
   }

}


namespace Botan {

#if defined(__SIZEOF_INT128__)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned __int128 uint128_t;

#elif (BOTAN_GCC_VERSION > 440) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned int uint128_t __attribute__((mode(TI)));
#endif

}

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi)      \
   do {                                      \
      const uint128_t r = static_cast<uint128_t>(a) * b;   \
      *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;  \
      *lo = (r      ) & 0xFFFFFFFFFFFFFFFF;  \
   } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)

#include <intrin.h>
#pragma intrinsic(_umul128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) \
   do { *lo = _umul128(a, b, hi); } while(0)

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                           \
   asm("mulq %3" : "=d" (*hi), "=a" (*lo) : "a" (a), "rm" (b) : "cc"); \
   } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {              \
   asm("umulh %1,%2,%0" : "=r" (*hi) : "r" (a), "r" (b)); \
   *lo = a * b;                                           \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                \
   asm("xmpy.hu %0=%1,%2" : "=f" (*hi) : "f" (a), "f" (b)); \
   *lo = a * b;                                             \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                      \
   asm("mulhdu %0,%1,%2" : "=r" (*hi) : "r" (a), "r" (b) : "cc"); \
   *lo = a * b;                                                   \
} while(0)

#endif

#endif

namespace Botan {

/**
* Perform a 64x64->128 bit multiplication
*/
inline void mul64x64_128(u64bit a, u64bit b, u64bit* lo, u64bit* hi)
   {
#if defined(BOTAN_FAST_64X64_MUL)
   BOTAN_FAST_64X64_MUL(a, b, lo, hi);
#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts. Last resort for CPUs like UltraSPARC (with
   * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
   */
   const size_t HWORD_BITS = 32;
   const u32bit HWORD_MASK = 0xFFFFFFFF;

   const u32bit a_hi = (a >> HWORD_BITS);
   const u32bit a_lo = (a  & HWORD_MASK);
   const u32bit b_hi = (b >> HWORD_BITS);
   const u32bit b_lo = (b  & HWORD_MASK);

   u64bit x0 = static_cast<u64bit>(a_hi) * b_hi;
   u64bit x1 = static_cast<u64bit>(a_lo) * b_hi;
   u64bit x2 = static_cast<u64bit>(a_hi) * b_lo;
   u64bit x3 = static_cast<u64bit>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
   x2 += x3 >> HWORD_BITS;

   // this one can overflow
   x2 += x1;

   // propagate the carry if any
   x0 += static_cast<u64bit>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

   *hi = x0 + (x2 >> HWORD_BITS);
   *lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
   }

}


namespace Botan {

#if (BOTAN_MP_WORD_BITS == 8)
  typedef byte word;
  typedef u16bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef u16bit word;
  typedef u32bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 32)
  typedef u32bit word;
  typedef u64bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 64)
  typedef u64bit word;

  #if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
    typedef uint128_t dword;
    #define BOTAN_HAS_MP_DWORD
  #endif

#else
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

}


namespace Botan {

/**
* Arbitrary precision integer
*/
class BOTAN_DLL BigInt
   {
   public:
     /**
     * Base enumerator for encoding and decoding
     */
     enum Base { Decimal = 10, Hexadecimal = 16, Binary = 256 };

     /**
     * Sign symbol definitions for positive and negative numbers
     */
     enum Sign { Negative = 0, Positive = 1 };

     /**
     * DivideByZero Exception
     */
     struct BOTAN_DLL DivideByZero : public Exception
        { DivideByZero() : Exception("BigInt divide by zero") {} };

     /**
     * Create empty BigInt
     */
     BigInt() { m_signedness = Positive; }

     /**
     * Create BigInt from 64 bit integer
     * @param n initial value of this BigInt
     */
     BigInt(u64bit n);

     /**
     * Copy Constructor
     * @param other the BigInt to copy
     */
     BigInt(const BigInt& other);

     /**
     * Create BigInt from a string. If the string starts with 0x the
     * rest of the string will be interpreted as hexadecimal digits.
     * Otherwise, it will be interpreted as a decimal number.
     *
     * @param str the string to parse for an integer value
     */
     BigInt(const std::string& str);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the byte array holding the value
     * @param length size of buf
     * @param base is the number base of the integer in buf
     */
     BigInt(const byte buf[], size_t length, Base base = Binary);

     /**
     * Create a random BigInt of the specified size
     * @param rng random number generator
     * @param bits size in bits
     */
     BigInt(RandomNumberGenerator& rng, size_t bits);

     /**
     * Create BigInt of specified size, all zeros
     * @param sign the sign
     * @param n size of the internal register in words
     */
     BigInt(Sign sign, size_t n);

     /**
     * Move constructor
     */
     BigInt(BigInt&& other)
        {
        this->swap(other);
        }

     /**
     * Move assignment
     */
     BigInt& operator=(BigInt&& other)
        {
        if(this != &other)
           this->swap(other);

        return (*this);
        }

     /**
     * Copy assignment
     */
     BigInt& operator=(const BigInt&) = default;

     /**
     * Swap this value with another
     * @param other BigInt to swap values with
     */
     void swap(BigInt& other)
        {
        m_reg.swap(other.m_reg);
        std::swap(m_signedness, other.m_signedness);
        }

     void swap_reg(secure_vector<word>& reg)
        {
        m_reg.swap(reg);
        }

     /**
     * += operator
     * @param y the BigInt to add to this
     */
     BigInt& operator+=(const BigInt& y);

     /**
     * -= operator
     * @param y the BigInt to subtract from this
     */
     BigInt& operator-=(const BigInt& y);

     /**
     * *= operator
     * @param y the BigInt to multiply with this
     */
     BigInt& operator*=(const BigInt& y);

     /**
     * /= operator
     * @param y the BigInt to divide this by
     */
     BigInt& operator/=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus to reduce this by
     */
     BigInt& operator%=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus (word) to reduce this by
     */
     word    operator%=(word y);

     /**
     * Left shift operator
     * @param shift the number of bits to shift this left by
     */
     BigInt& operator<<=(size_t shift);

     /**
     * Right shift operator
     * @param shift the number of bits to shift this right by
     */
     BigInt& operator>>=(size_t shift);

     /**
     * Increment operator
     */
     BigInt& operator++() { return (*this += 1); }

     /**
     * Decrement operator
     */
     BigInt& operator--() { return (*this -= 1); }

     /**
     * Postfix increment operator
     */
     BigInt  operator++(int) { BigInt x = (*this); ++(*this); return x; }

     /**
     * Postfix decrement operator
     */
     BigInt  operator--(int) { BigInt x = (*this); --(*this); return x; }

     /**
     * Unary negation operator
     * @return negative this
     */
     BigInt operator-() const;

     /**
     * ! operator
     * @return true iff this is zero, otherwise false
     */
     bool operator !() const { return (!is_nonzero()); }

     /**
     * Zeroize the BigInt. The size of the underlying register is not
     * modified.
     */
     void clear() { zeroise(m_reg); }

     /**
     * Compare this to another BigInt
     * @param n the BigInt value to compare with
     * @param check_signs include sign in comparison?
     * @result if (this<n) return -1, if (this>n) return 1, if both
     * values are identical return 0 [like Perl's <=> operator]
     */
     s32bit cmp(const BigInt& n, bool check_signs = true) const;

     /**
     * Test if the integer has an even value
     * @result true if the integer is even, false otherwise
     */
     bool is_even() const { return (get_bit(0) == 0); }

     /**
     * Test if the integer has an odd value
     * @result true if the integer is odd, false otherwise
     */
     bool is_odd()  const { return (get_bit(0) == 1); }

     /**
     * Test if the integer is not zero
     * @result true if the integer is non-zero, false otherwise
     */
     bool is_nonzero() const { return (!is_zero()); }

     /**
     * Test if the integer is zero
     * @result true if the integer is zero, false otherwise
     */
     bool is_zero() const
        {
        const size_t sw = sig_words();

        for(size_t i = 0; i != sw; ++i)
           if(m_reg[i])
              return false;
        return true;
        }

     /**
     * Set bit at specified position
     * @param n bit position to set
     */
     void set_bit(size_t n);

     /**
     * Clear bit at specified position
     * @param n bit position to clear
     */
     void clear_bit(size_t n);

     /**
     * Clear all but the lowest n bits
     * @param n amount of bits to keep
     */
     void mask_bits(size_t n)
        {
        if(n == 0) { clear(); return; }

        const size_t top_word = n / BOTAN_MP_WORD_BITS;
        const word mask = (static_cast<word>(1) << (n % BOTAN_MP_WORD_BITS)) - 1;

        if(top_word < size())
           {
           clear_mem(&m_reg[top_word+1], size() - (top_word + 1));
           m_reg[top_word] &= mask;
           }
        }

     /**
     * Return bit value at specified position
     * @param n the bit offset to test
     * @result true, if the bit at position n is set, false otherwise
     */
     bool get_bit(size_t n) const
        {
        return ((word_at(n / BOTAN_MP_WORD_BITS) >> (n % BOTAN_MP_WORD_BITS)) & 1);
        }

     /**
     * Return (a maximum of) 32 bits of the complete value
     * @param offset the offset to start extracting
     * @param length amount of bits to extract (starting at offset)
     * @result the integer extracted from the register starting at
     * offset with specified length
     */
     u32bit get_substring(size_t offset, size_t length) const;

     /**
     * Convert this value into a u32bit, if it is in the range
     * [0 ... 2**32-1], or otherwise throw an exception.
     * @result the value as a u32bit if conversion is possible
     */
     u32bit to_u32bit() const;

     /**
     * @param n the offset to get a byte from
     * @result byte at offset n
     */
     byte byte_at(size_t n) const
        {
        return get_byte(sizeof(word) - (n % sizeof(word)) - 1,
                        word_at(n / sizeof(word)));
        }

     /**
     * Return the word at a specified position of the internal register
     * @param n position in the register
     * @return value at position n
     */
     word word_at(size_t n) const
        { return ((n < size()) ? m_reg[n] : 0); }

     void set_word_at(size_t i, word w)
        {
        grow_to(i + 1);
        m_reg[i] = w;
        }

     /**
     * Tests if the sign of the integer is negative
     * @result true, iff the integer has a negative sign
     */
     bool is_negative() const { return (sign() == Negative); }

     /**
     * Tests if the sign of the integer is positive
     * @result true, iff the integer has a positive sign
     */
     bool is_positive() const { return (sign() == Positive); }

     /**
     * Return the sign of the integer
     * @result the sign of the integer
     */
     Sign sign() const { return (m_signedness); }

     /**
     * @result the opposite sign of the represented integer value
     */
     Sign reverse_sign() const;

     /**
     * Flip the sign of this BigInt
     */
     void flip_sign();

     /**
     * Set sign of the integer
     * @param sign new Sign to set
     */
     void set_sign(Sign sign);

     /**
     * @result absolute (positive) value of this
     */
     BigInt abs() const;

     /**
     * Give size of internal register
     * @result size of internal register in words
     */
     size_t size() const { return m_reg.size(); }

     /**
     * Return how many words we need to hold this value
     * @result significant words of the represented integer value
     */
     size_t sig_words() const
        {
        const word* x = &m_reg[0];
        size_t sig = m_reg.size();

        while(sig && (x[sig-1] == 0))
           sig--;
        return sig;
        }

     /**
     * Give byte length of the integer
     * @result byte length of the represented integer value
     */
     size_t bytes() const { return (bits() + 7) / 8; }

     /**
     * Get the bit length of the integer
     * @result bit length of the represented integer value
     */
     size_t bits() const;

     /**
     * Return a mutable pointer to the register
     * @result a pointer to the start of the internal register
     */
     word* mutable_data() { return &m_reg[0]; }

     /**
     * Return a const pointer to the register
     * @result a pointer to the start of the internal register
     */
     const word* data() const { return &m_reg[0]; }

     secure_vector<word>& get_word_vector() { return m_reg; }
     const secure_vector<word>& get_word_vector() const { return m_reg; }

     /**
     * Increase internal register buffer to at least n words
     * @param n new size of register
     */
     void grow_to(size_t n)
        {
        if(n > size())
           m_reg.resize(n + (8 - n % 8));
        }

     /**
     * Fill BigInt with a random number with size of bitsize
     * @param rng the random number generator to use
     * @param bitsize number of bits the created random value should have
     */
     void randomize(RandomNumberGenerator& rng, size_t bitsize = 0);

     /**
     * Store BigInt-value in a given byte array
     * @param buf destination byte array for the integer value
     */
     void binary_encode(byte buf[]) const;

     /**
     * Read integer value from a byte array with given size
     * @param buf byte array buffer containing the integer
     * @param length size of buf
     */
     void binary_decode(const byte buf[], size_t length);

     /**
     * Read integer value from a byte array (secure_vector<byte>)
     * @param buf the array to load from
     */
     void binary_decode(const secure_vector<byte>& buf)
        {
        binary_decode(&buf[0], buf.size());
        }

     /**
     * @param base the base to measure the size for
     * @return size of this integer in base base
     */
     size_t encoded_size(Base base = Binary) const;

     /**
     * @param rng a random number generator
     * @param min the minimum value
     * @param max the maximum value
     * @return random integer in [min,max)
     */
     static BigInt random_integer(RandomNumberGenerator& rng,
                                  const BigInt& min,
                                  const BigInt& max);

     /**
     * Create a power of two
     * @param n the power of two to create
     * @return bigint representing 2^n
     */
     static BigInt power_of_2(size_t n)
        {
        BigInt b;
        b.set_bit(n);
        return b;
        }

     /**
     * Encode the integer value from a BigInt to a std::vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static std::vector<byte> encode(const BigInt& n, Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a secure_vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static secure_vector<byte> encode_locked(const BigInt& n,
                                              Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a byte array
     * @param buf destination byte array for the encoded integer
     * value with given base
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     */
     static void encode(byte buf[], const BigInt& n, Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param length size of buf
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const byte buf[], size_t length,
                          Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const secure_vector<byte>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(&buf[0], buf.size(), base);
        }

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const std::vector<byte>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(&buf[0], buf.size(), base);
        }

     /**
     * Encode a BigInt to a byte array according to IEEE 1363
     * @param n the BigInt to encode
     * @param bytes the length of the resulting secure_vector<byte>
     * @result a secure_vector<byte> containing the encoded BigInt
     */
     static secure_vector<byte> encode_1363(const BigInt& n, size_t bytes);

   private:
      secure_vector<word> m_reg;
      Sign m_signedness = Positive;
   };

/*
* Arithmetic Operators
*/
BigInt BOTAN_DLL operator+(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator-(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator*(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator/(const BigInt& x, const BigInt& d);
BigInt BOTAN_DLL operator%(const BigInt& x, const BigInt& m);
word   BOTAN_DLL operator%(const BigInt& x, word m);
BigInt BOTAN_DLL operator<<(const BigInt& x, size_t n);
BigInt BOTAN_DLL operator>>(const BigInt& x, size_t n);

/*
* Comparison Operators
*/
inline bool operator==(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) == 0); }
inline bool operator!=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) != 0); }
inline bool operator<=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) <= 0); }
inline bool operator>=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) >= 0); }
inline bool operator<(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) < 0); }
inline bool operator>(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) > 0); }

/*
* I/O Operators
*/
BOTAN_DLL std::ostream& operator<<(std::ostream&, const BigInt&);
BOTAN_DLL std::istream& operator>>(std::istream&, BigInt&);

}

namespace std {

template<>
inline void swap<Botan::BigInt>(Botan::BigInt& x, Botan::BigInt& y)
   {
   x.swap(y);
   }

}


namespace Botan {

/**
* Modular Exponentiator Interface
*/
class BOTAN_DLL Modular_Exponentiator
   {
   public:
      virtual void set_base(const BigInt&) = 0;
      virtual void set_exponent(const BigInt&) = 0;
      virtual BigInt execute() const = 0;
      virtual Modular_Exponentiator* copy() const = 0;
      virtual ~Modular_Exponentiator() {}
   };

/**
* Modular Exponentiator Proxy
*/
class BOTAN_DLL Power_Mod
   {
   public:

      enum Usage_Hints {
         NO_HINTS        = 0x0000,

         BASE_IS_FIXED   = 0x0001,
         BASE_IS_SMALL   = 0x0002,
         BASE_IS_LARGE   = 0x0004,
         BASE_IS_2       = 0x0008,

         EXP_IS_FIXED    = 0x0100,
         EXP_IS_SMALL    = 0x0200,
         EXP_IS_LARGE    = 0x0400
      };

      /*
      * Try to choose a good window size
      */
      static size_t window_bits(size_t exp_bits, size_t base_bits,
                                Power_Mod::Usage_Hints hints);

      void set_modulus(const BigInt&, Usage_Hints = NO_HINTS) const;
      void set_base(const BigInt&) const;
      void set_exponent(const BigInt&) const;

      BigInt execute() const;

      Power_Mod& operator=(const Power_Mod&);

      Power_Mod(const BigInt& = 0, Usage_Hints = NO_HINTS);
      Power_Mod(const Power_Mod&);
      virtual ~Power_Mod();
   private:
      mutable Modular_Exponentiator* core;
   };

/**
* Fixed Exponent Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Exponent_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& b) const
         { set_base(b); return execute(); }

      Fixed_Exponent_Power_Mod() {}

      Fixed_Exponent_Power_Mod(const BigInt& exponent,
                               const BigInt& modulus,
                               Usage_Hints hints = NO_HINTS);
   };

/**
* Fixed Base Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Base_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& e) const
         { set_exponent(e); return execute(); }

      Fixed_Base_Power_Mod() {}

      Fixed_Base_Power_Mod(const BigInt& base,
                           const BigInt& modulus,
                           Usage_Hints hints = NO_HINTS);
   };

}


namespace Botan {

/**
* Fused multiply-add
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a*b)+c
*/
BigInt BOTAN_DLL mul_add(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Fused subtract-multiply
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a-b)*c
*/
BigInt BOTAN_DLL sub_mul(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Return the absolute value
* @param n an integer
* @return absolute value of n
*/
inline BigInt abs(const BigInt& n) { return n.abs(); }

/**
* Compute the greatest common divisor
* @param x a positive integer
* @param y a positive integer
* @return gcd(x,y)
*/
BigInt BOTAN_DLL gcd(const BigInt& x, const BigInt& y);

/**
* Least common multiple
* @param x a positive integer
* @param y a positive integer
* @return z, smallest integer such that z % x == 0 and z % y == 0
*/
BigInt BOTAN_DLL lcm(const BigInt& x, const BigInt& y);

/**
* @param x an integer
* @return (x*x)
*/
BigInt BOTAN_DLL square(const BigInt& x);

/**
* Modular inversion
* @param x a positive integer
* @param modulus a positive integer
* @return y st (x*y) % modulus == 1
*/
BigInt BOTAN_DLL inverse_mod(const BigInt& x,
                             const BigInt& modulus);

/**
* Compute the Jacobi symbol. If n is prime, this is equivalent
* to the Legendre symbol.
* @see http://mathworld.wolfram.com/JacobiSymbol.html
*
* @param a is a non-negative integer
* @param n is an odd integer > 1
* @return (n / m)
*/
s32bit BOTAN_DLL jacobi(const BigInt& a,
                        const BigInt& n);

/**
* Modular exponentation
* @param b an integer base
* @param x a positive exponent
* @param m a positive modulus
* @return (b^x) % m
*/
BigInt BOTAN_DLL power_mod(const BigInt& b,
                           const BigInt& x,
                           const BigInt& m);

/**
* Compute the square root of x modulo a prime using the
* Shanks-Tonnelli algorithm
*
* @param x the input
* @param p the prime
* @return y such that (y*y)%p == x, or -1 if no such integer
*/
BigInt BOTAN_DLL ressol(const BigInt& x, const BigInt& p);

/*
* Compute -input^-1 mod 2^MP_WORD_BITS. Returns zero if input
* is even. If input is odd, input and 2^n are relatively prime
* and an inverse exists.
*/
word BOTAN_DLL monty_inverse(word input);

/**
* @param x a positive integer
* @return count of the zero bits in x, or, equivalently, the largest
*         value of n such that 2^n divides x evenly. Returns zero if
*         n is less than or equal to zero.
*/
size_t BOTAN_DLL low_zero_bits(const BigInt& x);

/**
* Check for primality
* @param n a positive integer to test for primality
* @param rng a random number generator
* @param prob chance of false positive is bounded by 1/2**prob
* @param is_random true if n was randomly chosen by us
* @return true if all primality tests passed, otherwise false
*/
bool BOTAN_DLL is_prime(const BigInt& n,
                        RandomNumberGenerator& rng,
                        size_t prob = 56,
                        bool is_random = false);

inline bool quick_check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 32); }

inline bool check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 56); }

inline bool verify_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 80); }


/**
* Randomly generate a prime
* @param rng a random number generator
* @param bits how large the resulting prime should be in bits
* @param coprime a positive integer the result should be coprime to
* @param equiv a non-negative number that the result should be
               equivalent to modulo equiv_mod
* @param equiv_mod the modulus equiv should be checked against
* @return random prime with the specified criteria
*/
BigInt BOTAN_DLL random_prime(RandomNumberGenerator& rng,
                              size_t bits, const BigInt& coprime = 1,
                              size_t equiv = 1, size_t equiv_mod = 2);

/**
* Return a 'safe' prime, of the form p=2*q+1 with q prime
* @param rng a random number generator
* @param bits is how long the resulting prime should be
* @return prime randomly chosen from safe primes of length bits
*/
BigInt BOTAN_DLL random_safe_prime(RandomNumberGenerator& rng,
                                   size_t bits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param af an algorithm factory
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @return random seed used to generate this parameter set
*/
std::vector<byte> BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param af an algorithm factory
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @param seed the seed used to generate the parameters
* @return true if seed generated a valid DSA parameter set, otherwise
          false. p_out and q_out are only valid if true was returned.
*/
bool BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits,
                    const std::vector<byte>& seed);

/**
* The size of the PRIMES[] array
*/
const size_t PRIME_TABLE_SIZE = 6541;

/**
* A const array of all primes less than 65535
*/
extern const u16bit BOTAN_DLL PRIMES[];

}


namespace Botan {

/**
* Modular Reducer (using Barrett's technique)
*/
class BOTAN_DLL Modular_Reducer
   {
   public:
      const BigInt& get_modulus() const { return modulus; }

      BigInt reduce(const BigInt& x) const;

      /**
      * Multiply mod p
      * @param x
      * @param y
      * @return (x * y) % p
      */
      BigInt multiply(const BigInt& x, const BigInt& y) const
         { return reduce(x * y); }

      /**
      * Square mod p
      * @param x
      * @return (x * x) % p
      */
      BigInt square(const BigInt& x) const
         { return reduce(Botan::square(x)); }

      /**
      * Cube mod p
      * @param x
      * @return (x * x * x) % p
      */
      BigInt cube(const BigInt& x) const
         { return multiply(x, this->square(x)); }

      bool initialized() const { return (mod_words != 0); }

      Modular_Reducer() { mod_words = 0; }
      Modular_Reducer(const BigInt& mod);
   private:
      BigInt modulus, modulus_2, mu;
      size_t mod_words;
   };

}


namespace Botan {

/**
* Blinding Function Object
*/
class BOTAN_DLL Blinder
   {
   public:
      BigInt blind(const BigInt& x) const;

      BigInt unblind(const BigInt& x) const;

      bool initialized() const { return m_reducer.initialized(); }

      Blinder() {}

      Blinder(const BigInt& modulus,
              std::function<BigInt (const BigInt&)> fwd_func,
              std::function<BigInt (const BigInt&)> inv_func);

   private:
      Modular_Reducer m_reducer;
      mutable BigInt m_e, m_d;
   };

}


namespace Botan {

/**
* Blowfish
*/
class BOTAN_DLL Blowfish : public Block_Cipher_Fixed_Params<8, 1, 56>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      /**
      * Modified EKSBlowfish key schedule, used for bcrypt password hashing
      */
      void eks_key_schedule(const byte key[], size_t key_length,
                            const byte salt[16], size_t workfactor);

      void clear();
      std::string name() const { return "Blowfish"; }
      BlockCipher* clone() const { return new Blowfish; }
   private:
      void key_schedule(const byte key[], size_t length);

      void key_expansion(const byte key[],
                         size_t key_length,
                         const byte salt[16]);

      void generate_sbox(secure_vector<u32bit>& box,
                         u32bit& L, u32bit& R,
                         const byte salt[16],
                         size_t salt_off) const;

      static const u32bit P_INIT[18];
      static const u32bit S_INIT[1024];

      secure_vector<u32bit> S, P;
   };

}


namespace Botan {

class BlockCipher;
class StreamCipher;
class HashFunction;
class MessageAuthenticationCode;
class PBKDF;

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
*
* @param algo_spec the name of the desired block cipher
* @return pointer to the block cipher object
*/
BOTAN_DLL BlockCipher* get_block_cipher(const std::string& algo_spec,
                                        const std::string& provider = "");

BOTAN_DLL std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                         const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec);

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
BOTAN_DLL StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                          const std::string& provider = "");

BOTAN_DLL std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                           const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec);

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
BOTAN_DLL HashFunction* get_hash_function(const std::string& algo_spec,
                                          const std::string& provider = "");

BOTAN_DLL std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                           const std::string& provider = "");

inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return get_hash_function(algo_spec, provider);
   }

BOTAN_DLL std::vector<std::string> get_hash_function_providers(const std::string& algo_spec);

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
BOTAN_DLL MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "");

BOTAN_DLL std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_mac_providers(const std::string& algo_spec);

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL PBKDF* get_pbkdf(const std::string& algo_spec,
                           const std::string& provider = "");

}


namespace Botan {

/*
* Get information describing the version
*/

/**
* Get a human-readable string identifying the version of Botan.
* No particular format should be assumed.
* @return version string
*/
BOTAN_DLL std::string version_string();

BOTAN_DLL const char* version_cstr();

/**
* Return the date this version of botan was released, in an integer of
* the form YYYYMMDD. For instance a version released on May 21, 2013
* would return the integer 20130521. If the currently running version
* is not an official release, this function will return 0 instead.
*
* @return release date, or zero if unreleased
*/
BOTAN_DLL u32bit version_datestamp();

/**
* Get the major version number.
* @return major version number
*/
BOTAN_DLL u32bit version_major();

/**
* Get the minor version number.
* @return minor version number
*/
BOTAN_DLL u32bit version_minor();

/**
* Get the patch number.
* @return patch number
*/
BOTAN_DLL u32bit version_patch();

/*
* Macros for compile-time version checks
*/
#define BOTAN_VERSION_CODE_FOR(a,b,c) ((a << 16) | (b << 8) | (c))

/**
* Compare using BOTAN_VERSION_CODE_FOR, as in
*  # if BOTAN_VERSION_CODE < BOTAN_VERSION_CODE_FOR(1,8,0)
*  #    error "Botan version too old"
*  # endif
*/
#define BOTAN_VERSION_CODE BOTAN_VERSION_CODE_FOR(BOTAN_VERSION_MAJOR, \
                                                  BOTAN_VERSION_MINOR, \
                                                  BOTAN_VERSION_PATCH)

}



#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif


namespace Botan {

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
template<typename T> inline T rotate_left(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
   }

/**
* Bit rotation right
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated right by rot bits
*/
template<typename T> inline T rotate_right(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
   }

}


#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)
  #include <emmintrin.h>
#endif

namespace Botan {

/**
* Swap a 16 bit integer
*/
inline u16bit reverse_bytes(u16bit val)
   {
   return rotate_left(val, 8);
   }

/**
* Swap a 32 bit integer
*/
inline u32bit reverse_bytes(u32bit val)
   {
#if BOTAN_GCC_VERSION >= 430 && !defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
   /*
   GCC intrinsic added in 4.3, works for a number of CPUs

   However avoid under ARM, as it branches to a function in libgcc
   instead of generating inline asm, so slower even than the generic
   rotate version below.
   */
   return __builtin_bswap32(val);

#elif BOTAN_USE_GCC_INLINE_ASM && defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   // GCC-style inline assembly for x86 or x86-64
   asm("bswapl %0" : "=r" (val) : "0" (val));
   return val;

#elif BOTAN_USE_GCC_INLINE_ASM && defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)

   asm ("eor r3, %1, %1, ror #16\n\t"
        "bic r3, r3, #0x00FF0000\n\t"
        "mov %0, %1, ror #8\n\t"
        "eor %0, %0, r3, lsr #8"
        : "=r" (val)
        : "0" (val)
        : "r3", "cc");

   return val;

#elif defined(_MSC_VER) && defined(BOTAN_TARGET_ARCH_IS_X86_32)

   // Visual C++ inline asm for 32-bit x86, by Yves Jerschow
   __asm mov eax, val;
   __asm bswap eax;

#else

   // Generic implementation
   return (rotate_right(val, 8) & 0xFF00FF00) |
          (rotate_left (val, 8) & 0x00FF00FF);

#endif
   }

/**
* Swap a 64 bit integer
*/
inline u64bit reverse_bytes(u64bit val)
   {
#if BOTAN_GCC_VERSION >= 430

   // GCC intrinsic added in 4.3, works for a number of CPUs
   return __builtin_bswap64(val);

#elif BOTAN_USE_GCC_INLINE_ASM && defined(BOTAN_TARGET_ARCH_IS_X86_64)
   // GCC-style inline assembly for x86-64
   asm("bswapq %0" : "=r" (val) : "0" (val));
   return val;

#else
   /* Generic implementation. Defined in terms of 32-bit bswap so any
    * optimizations in that version can help here (particularly
    * useful for 32-bit x86).
    */

   u32bit hi = static_cast<u32bit>(val >> 32);
   u32bit lo = static_cast<u32bit>(val);

   hi = reverse_bytes(hi);
   lo = reverse_bytes(lo);

   return (static_cast<u64bit>(lo) << 32) | hi;
#endif
   }

/**
* Swap 4 Ts in an array
*/
template<typename T>
inline void bswap_4(T x[4])
   {
   x[0] = reverse_bytes(x[0]);
   x[1] = reverse_bytes(x[1]);
   x[2] = reverse_bytes(x[2]);
   x[3] = reverse_bytes(x[3]);
   }

#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)

/**
* Swap 4 u32bits in an array using SSE2 shuffle instructions
*/
template<>
inline void bswap_4(u32bit x[4])
   {
   __m128i T = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));

   T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
   T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

   T =  _mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8));

   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), T);
   }

#endif

}


namespace Botan {

/**
* Filter mixin that breaks input into blocks, useful for
* cipher modes
*/
class BOTAN_DLL Buffered_Filter
   {
   public:
      /**
      * Write bytes into the buffered filter, which will them emit them
      * in calls to buffered_block in the subclass
      * @param in the input bytes
      * @param length of in in bytes
      */
      void write(const byte in[], size_t length);

      template<typename Alloc>
         void write(const std::vector<byte, Alloc>& in, size_t length)
         {
         write(&in[0], length);
         }

      /**
      * Finish a message, emitting to buffered_block and buffered_final
      * Will throw an exception if less than final_minimum bytes were
      * written into the filter.
      */
      void end_msg();

      /**
      * Initialize a Buffered_Filter
      * @param block_size the function buffered_block will be called
      *        with inputs which are a multiple of this size
      * @param final_minimum the function buffered_final will be called
      *        with at least this many bytes.
      */
      Buffered_Filter(size_t block_size, size_t final_minimum);

      virtual ~Buffered_Filter() {}
   protected:
      /**
      * The block processor, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be a multiple
      *        of block_size
      */
      virtual void buffered_block(const byte input[], size_t length) = 0;

      /**
      * The final block, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be at least
      *        final_minimum bytes
      */
      virtual void buffered_final(const byte input[], size_t length) = 0;

      /**
      * @return block size of inputs
      */
      size_t buffered_block_size() const { return main_block_mod; }

      /**
      * @return current position in the buffer
      */
      size_t current_position() const { return buffer_pos; }

      /**
      * Reset the buffer position
      */
      void buffer_reset() { buffer_pos = 0; }
   private:
      size_t main_block_mod, final_minimum;

      secure_vector<byte> buffer;
      size_t buffer_pos;
   };

}


namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   u32bit year;

   /** The month, 1 through 12 for Jan to Dec */
   byte month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   byte day;

   /** Hour in 24-hour form, 0 to 23 */
   byte hour;

   /** Minutes in the hour, 0 to 60 */
   byte minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   byte seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(u32bit y, byte mon, byte d, byte h, byte min, byte sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}
   };

/*
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}


namespace Botan {

/**
* Camellia-128
*/
class BOTAN_DLL Camellia_128 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-128"; }
      BlockCipher* clone() const { return new Camellia_128; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

/**
* Camellia-192
*/
class BOTAN_DLL Camellia_192 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-192"; }
      BlockCipher* clone() const { return new Camellia_192; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

/**
* Camellia-256
*/
class BOTAN_DLL Camellia_256 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-256"; }
      BlockCipher* clone() const { return new Camellia_256; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

}


namespace Botan {

/**
* Block Cipher Cascade
*/
class BOTAN_DLL Cascade_Cipher : public BlockCipher
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      size_t block_size() const { return m_block; }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(m_cipher1->maximum_keylength() +
                                         m_cipher2->maximum_keylength());
         }

      void clear();
      std::string name() const;
      BlockCipher* clone() const;

      static Cascade_Cipher* make(const Spec& spec);

      /**
      * Create a cascade of two block ciphers
      * @param cipher1 the first cipher
      * @param cipher2 the second cipher
      */
      Cascade_Cipher(BlockCipher* cipher1, BlockCipher* cipher2);

      Cascade_Cipher(const Cascade_Cipher&) = delete;
      Cascade_Cipher& operator=(const Cascade_Cipher&) = delete;
   private:
      void key_schedule(const byte[], size_t);

      size_t m_block;
      std::unique_ptr<BlockCipher> m_cipher1, m_cipher2;
   };


}


namespace Botan {

/**
* CAST-128
*/
class BOTAN_DLL CAST_128 : public Block_Cipher_Fixed_Params<8, 11, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "CAST-128"; }
      BlockCipher* clone() const { return new CAST_128; }

   private:
      void key_schedule(const byte[], size_t);

      static void cast_ks(secure_vector<u32bit>& ks,
                          secure_vector<u32bit>& user_key);

      secure_vector<u32bit> MK;
      secure_vector<byte> RK;
   };

}


namespace Botan {

/**
* CAST-256
*/
class BOTAN_DLL CAST_256 : public Block_Cipher_Fixed_Params<16, 4, 32, 4>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "CAST-256"; }
      BlockCipher* clone() const { return new CAST_256; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> MK;
      secure_vector<byte> RK;
   };

}


namespace Botan {

/**
* Block Cipher Mode Padding Method
* This class is pretty limited, it cannot deal well with
* randomized padding methods, or any padding method that
* wants to add more than one block. For instance, it should
* be possible to define cipher text stealing mode as simply
* a padding mode for CBC, which happens to consume the last
* two block (and requires use of the block cipher).
*/
class BOTAN_DLL BlockCipherModePaddingMethod
   {
   public:
      virtual void add_padding(secure_vector<byte>& buffer,
                               size_t final_block_bytes,
                               size_t block_size) const = 0;

      /**
      * @param block the last block
      * @param size the of the block
      */
      virtual size_t unpad(const byte block[],
                           size_t size) const = 0;

      /**
      * @param block_size of the cipher
      * @return valid block size for this padding mode
      */
      virtual bool valid_blocksize(size_t block_size) const = 0;

      /**
      * @return name of the mode
      */
      virtual std::string name() const = 0;

      /**
      * virtual destructor
      */
      virtual ~BlockCipherModePaddingMethod() {}
   };

/**
* PKCS#7 Padding
*/
class BOTAN_DLL PKCS7_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0 && bs < 256); }

      std::string name() const { return "PKCS7"; }
   };

/**
* ANSI X9.23 Padding
*/
class BOTAN_DLL ANSI_X923_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0 && bs < 256); }

      std::string name() const { return "X9.23"; }
   };

/**
* One And Zeros Padding
*/
class BOTAN_DLL OneAndZeros_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0); }

      std::string name() const { return "OneAndZeros"; }
   };

/**
* Null Padding
*/
class BOTAN_DLL Null_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>&, size_t, size_t) const override {}

      size_t unpad(const byte[], size_t size) const { return size; }

      bool valid_blocksize(size_t) const { return true; }

      std::string name() const { return "NoPadding"; }
   };

BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec);

}


namespace Botan {

/**
* CBC Mode
*/
class BOTAN_DLL CBC_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      CBC_Mode(BlockCipher* cipher, BlockCipherModePaddingMethod* padding);

      const BlockCipher& cipher() const { return *m_cipher; }

      const BlockCipherModePaddingMethod& padding() const
         {
         BOTAN_ASSERT_NONNULL(m_padding);
         return *m_padding;
         }

      secure_vector<byte>& state() { return m_state; }

      byte* state_ptr() { return &m_state[0]; }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<BlockCipherModePaddingMethod> m_padding;
      secure_vector<byte> m_state;
   };

/**
* CBC Encryption
*/
class BOTAN_DLL CBC_Encryption : public CBC_Mode
   {
   public:
      CBC_Encryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   };

/**
* CBC Encryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Encryption : public CBC_Encryption
   {
   public:
      CTS_Encryption(BlockCipher* cipher) : CBC_Encryption(cipher, nullptr) {}

      size_t output_length(size_t input_length) const override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const;
   };

/**
* CBC Decryption
*/
class BOTAN_DLL CBC_Decryption : public CBC_Mode
   {
   public:
      CBC_Decryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding), m_tempbuf(update_granularity()) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   private:
      secure_vector<byte> m_tempbuf;
   };

/**
* CBC Decryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Decryption : public CBC_Decryption
   {
   public:
      CTS_Decryption(BlockCipher* cipher) : CBC_Decryption(cipher, nullptr) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const;
   };

}


namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @param length the length of param in
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const byte in[], size_t length);

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual MessageAuthenticationCode* clone() const = 0;

      /**
      * Get the name of this algorithm.
      * @return name of this algorithm
      */
      virtual std::string name() const = 0;

      typedef SCAN_Name Spec;
   };

}


namespace Botan {

/**
* CBC-MAC
*/
class BOTAN_DLL CBC_MAC : public MessageAuthenticationCode
   {
   public:
      std::string name() const;
      MessageAuthenticationCode* clone() const;
      size_t output_length() const { return m_cipher->block_size(); }
      void clear();

      Key_Length_Specification key_spec() const
         {
         return m_cipher->key_spec();
         }

      /**
      * @param cipher the underlying block cipher to use
      */
      CBC_MAC(BlockCipher* cipher);

      static CBC_MAC* make(const Spec& spec);
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_state;
      size_t m_position = 0;
   };

}


namespace Botan {

/**
* Base class for CCM encryption and decryption
* @see RFC 3610
*/
class BOTAN_DLL CCM_Mode : public AEAD_Mode
   {
   public:
      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t default_nonce_length() const override;

      void clear() override;

      size_t tag_size() const { return m_tag_size; }

   protected:
      const size_t BS = 16; // intrinsic to CCM definition

      CCM_Mode(BlockCipher* cipher, size_t tag_size, size_t L);

      size_t L() const { return m_L; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void encode_length(size_t len, byte out[]);

      void inc(secure_vector<byte>& C);

      const secure_vector<byte>& ad_buf() const { return m_ad_buf; }

      secure_vector<byte>& msg_buf() { return m_msg_buf; }

      secure_vector<byte> format_b0(size_t msg_size);
      secure_vector<byte> format_c0();
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      const size_t m_tag_size;
      const size_t m_L;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_nonce, m_msg_buf, m_ad_buf;
   };

/**
* CCM Encryption
*/
class BOTAN_DLL CCM_Encryption : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Encryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* CCM Decryption
*/
class BOTAN_DLL CCM_Decryption : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Decryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}

namespace Botan {

enum class Certificate_Status_Code {
   VERIFIED = 0x00000000,
   OCSP_RESPONSE_GOOD,
   NO_REVOCATION_DATA,

   // Local policy failures
   SIGNATURE_METHOD_TOO_WEAK = 1000,
   UNTRUSTED_HASH,

   // Time problems
   CERT_NOT_YET_VALID = 2000,
   CERT_HAS_EXPIRED,
   OCSP_NOT_YET_VALID,
   OCSP_HAS_EXPIRED,
   CRL_NOT_YET_VALID,
   CRL_HAS_EXPIRED,

   // Chain generation problems
   CERT_ISSUER_NOT_FOUND = 3000,
   CANNOT_ESTABLISH_TRUST,

   // Validation errors
   POLICY_ERROR = 4000,
   INVALID_USAGE,
   CERT_CHAIN_TOO_LONG,
   CA_CERT_NOT_FOR_CERT_ISSUER,

   // Revocation errors
   CA_CERT_NOT_FOR_CRL_ISSUER,
   OCSP_CERT_NOT_LISTED,
   OCSP_BAD_STATUS,

   // Hard failures
   CERT_IS_REVOKED = 5000,
   CRL_BAD_SIGNATURE,
   SIGNATURE_ERROR,
};

}


namespace Botan {

/**
* Public Key Base Class.
*/
class BOTAN_DLL Public_Key
   {
   public:
      /**
      * Get the name of the underlying public key scheme.
      * @return name of the public key scheme
      */
      virtual std::string algo_name() const = 0;

      /**
      * Return the estimated strength of the underlying key against
      * the best currently known attack. Note that this ignores anything
      * but pure attacks against the key itself and do not take into
      * account padding schemes, usage mistakes, etc which might reduce
      * the strength. However it does suffice to provide an upper bound.
      *
      * @return estimated strength in bits
      */
      virtual size_t estimated_strength() const = 0;

      /**
      * Get the OID of the underlying public key scheme.
      * @return OID of the public key scheme
      */
      virtual OID get_oid() const;

      /**
      * Test the key values for consistency.
      * @param rng rng to use
      * @param strong whether to perform strong and lengthy version
      * of the test
      * @return true if the test is passed
      */
      virtual bool check_key(RandomNumberGenerator& rng,
                             bool strong) const = 0;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts in bits
      */
      virtual size_t message_part_size() const { return 0; }

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message size in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * @return X.509 AlgorithmIdentifier for this key
      */
      virtual AlgorithmIdentifier algorithm_identifier() const = 0;

      /**
      * @return X.509 subject key encoding for this key object
      */
      virtual std::vector<byte> x509_subject_public_key() const = 0;

      virtual ~Public_Key() {}
   protected:
      /**
      * Self-test after loading a key
      * @param rng a random number generator
      */
      virtual void load_check(RandomNumberGenerator& rng) const;
   };

/**
* Private Key Base Class
*/
class BOTAN_DLL Private_Key : public virtual Public_Key
   {
   public:
      /**
      * @return PKCS #8 private key encoding for this key object
      */
      virtual secure_vector<byte> pkcs8_private_key() const = 0;

      /**
      * @return PKCS #8 AlgorithmIdentifier for this key
      * Might be different from the X.509 identifier, but normally is not
      */
      virtual AlgorithmIdentifier pkcs8_algorithm_identifier() const
         { return algorithm_identifier(); }

   protected:
      /**
      * Self-test after loading a key
      * @param rng a random number generator
      */
      void load_check(RandomNumberGenerator& rng) const;

      /**
      * Self-test after generating a key
      * @param rng a random number generator
      */
      void gen_check(RandomNumberGenerator& rng) const;
   };

/**
* PK Secret Value Derivation Key
*/
class BOTAN_DLL PK_Key_Agreement_Key : public virtual Private_Key
   {
   public:
      /*
      * @return public component of this key
      */
      virtual std::vector<byte> public_value() const = 0;

      virtual ~PK_Key_Agreement_Key() {}
   };

/*
* Typedefs
*/
typedef PK_Key_Agreement_Key PK_KA_Key;
typedef Public_Key X509_PublicKey;
typedef Private_Key PKCS8_PrivateKey;

}


namespace Botan {

/**
* This class represents pipe objects.
* A set of filters can be placed into a pipe, and information flows
* through the pipe until it reaches the end, where the output is
* collected for retrieval.  If you're familiar with the Unix shell
* environment, this design will sound quite familiar.
*/
class BOTAN_DLL Pipe : public DataSource
   {
   public:
      /**
      * An opaque type that identifies a message in this Pipe
      */
      typedef size_t message_id;

      /**
      * Exception if you use an invalid message as an argument to
      * read, remaining, etc
      */
      struct BOTAN_DLL Invalid_Message_Number : public Invalid_Argument
         {
         /**
         * @param where the error occured
         * @param msg the invalid message id that was used
         */
         Invalid_Message_Number(const std::string& where, message_id msg) :
            Invalid_Argument("Pipe::" + where + ": Invalid message number " +
                             std::to_string(msg))
            {}
         };

      /**
      * A meta-id for whatever the last message is
      */
      static const message_id LAST_MESSAGE;

      /**
      * A meta-id for the default message (set with set_default_msg)
      */
      static const message_id DEFAULT_MESSAGE;

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the byte array to write
      * @param length the length of the byte array in
      */
      void write(const byte in[], size_t length);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the secure_vector containing the data to write
      */
      void write(const secure_vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the std::vector containing the data to write
      */
      void write(const std::vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the string containing the data to write
      */
      void write(const std::string& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the DataSource to read the data from
      */
      void write(DataSource& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in a single byte to be written
      */
      void write(byte in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the byte array containing the data to write
      * @param length the length of the byte array to write
      */
      void process_msg(const byte in[], size_t length);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const secure_vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const std::vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the string containing the data to write
      */
      void process_msg(const std::string& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the DataSource providing the data to write
      */
      void process_msg(DataSource& in);

      /**
      * Find out how many bytes are ready to read.
      * @param msg the number identifying the message
      * for which the information is desired
      * @return number of bytes that can still be read
      */
      size_t remaining(message_id msg = DEFAULT_MESSAGE) const;

      /**
      * Read the default message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      *
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length);

      /**
      * Read a specified message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @param msg the number identifying the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length, message_id msg);

      /**
      * Read a single byte from the pipe. Moves the internal offset so
      * that every call to read will return a new portion of the
      * message.
      *
      * @param output the byte to write the result to
      * @param msg the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte& output, message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return secure_vector holding the contents of the pipe
      */
      secure_vector<byte> read_all(message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return string holding the contents of the pipe
      */
      std::string read_all_as_string(message_id = DEFAULT_MESSAGE);

      /** Read from the default message but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length, size_t offset) const;

      /** Read from the specified message but do not modify the
      * internal offset. Consecutive calls to peek() will return
      * portions of the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length,
                  size_t offset, message_id msg) const;

      /** Read a single byte from the specified message but do not
      * modify the internal offset. Consecutive calls to peek() will
      * return portions of the message starting at the same position.
      * @param output the byte to write the peeked message byte to
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte& output, size_t offset,
                  message_id msg = DEFAULT_MESSAGE) const;

      /**
      * @return the number of bytes read from the default message.
      */
      size_t get_bytes_read() const;

      /**
      * @return the number of bytes read from the specified message.
      */
      size_t get_bytes_read(message_id msg = DEFAULT_MESSAGE) const;

      /**
      * @return currently set default message
      */
      size_t default_msg() const { return default_read; }

      /**
      * Set the default message
      * @param msg the number identifying the message which is going to
      * be the new default message
      */
      void set_default_msg(message_id msg);

      /**
      * Get the number of messages the are in this pipe.
      * @return number of messages the are in this pipe
      */
      message_id message_count() const;

      /**
      * Test whether this pipe has any data that can be read from.
      * @return true if there is more data to read, false otherwise
      */
      bool end_of_data() const;

      /**
      * Start a new message in the pipe. A potential other message in this pipe
      * must be closed with end_msg() before this function may be called.
      */
      void start_msg();

      /**
      * End the current message.
      */
      void end_msg();

      /**
      * Insert a new filter at the front of the pipe
      * @param filt the new filter to insert
      */
      void prepend(Filter* filt);

      /**
      * Insert a new filter at the back of the pipe
      * @param filt the new filter to insert
      */
      void append(Filter* filt);

      /**
      * Remove the first filter at the front of the pipe.
      */
      void pop();

      /**
      * Reset this pipe to an empty pipe.
      */
      void reset();

      /**
      * Construct a Pipe of up to four filters. The filters are set up
      * in the same order as the arguments.
      */
      Pipe(Filter* = nullptr, Filter* = nullptr,
           Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Pipe from a list of filters
      * @param filters the set of filters to use
      */
      Pipe(std::initializer_list<Filter*> filters);

      Pipe(const Pipe&) = delete;
      Pipe& operator=(const Pipe&) = delete;

      ~Pipe();
   private:
      void init();
      void destruct(Filter*);
      void find_endpoints(Filter*);
      void clear_endpoints(Filter*);

      message_id get_message_no(const std::string&, message_id) const;

      Filter* pipe;
      class Output_Buffers* outputs;
      message_id default_read;
      bool inside_msg;
   };

/**
* Stream output operator; dumps the results from pipe's default
* message to the output stream.
* @param out an output stream
* @param pipe the pipe
*/
BOTAN_DLL std::ostream& operator<<(std::ostream& out, Pipe& pipe);

/**
* Stream input operator; dumps the remaining bytes of input
* to the (assumed open) pipe message.
* @param in the input stream
* @param pipe the pipe
*/
BOTAN_DLL std::istream& operator>>(std::istream& in, Pipe& pipe);

}

#if defined(BOTAN_HAS_PIPE_UNIXFD_IO)

namespace Botan {

/**
* Stream output operator; dumps the results from pipe's default
* message to the output stream.
* @param out file descriptor for an open output stream
* @param pipe the pipe
*/
int BOTAN_DLL operator<<(int out, Pipe& pipe);

/**
* File descriptor input operator; dumps the remaining bytes of input
* to the (assumed open) pipe message.
* @param in file descriptor for an open input stream
* @param pipe the pipe
*/
int BOTAN_DLL operator>>(int in, Pipe& pipe);

}

#endif


namespace Botan {

/**
* The two types of X509 encoding supported by Botan.
*/
enum X509_Encoding { RAW_BER, PEM };

/**
* This namespace contains functions for handling X.509 public keys
*/
namespace X509 {

/**
* BER encode a key
* @param key the public key to encode
* @return BER encoding of this key
*/
BOTAN_DLL std::vector<byte> BER_encode(const Public_Key& key);

/**
* PEM encode a public key into a string.
* @param key the key to encode
* @return PEM encoded key
*/
BOTAN_DLL std::string PEM_encode(const Public_Key& key);

/**
* Create a public key from a data source.
* @param source the source providing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(DataSource& source);

/**
* Create a public key from a file
* @param filename pathname to the file to load
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::string& filename);

/**
* Create a public key from a memory region.
* @param enc the memory region containing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::vector<byte>& enc);

/**
* Copy a key.
* @param key the public key to copy
* @return new public key object
*/
BOTAN_DLL Public_Key* copy_key(const Public_Key& key);

}

}


namespace Botan {

/**
* This class represents abstract X.509 signed objects as
* in the X.500 SIGNED macro
*/
class BOTAN_DLL X509_Object : public ASN1_Object
   {
   public:
      /**
      * The underlying data that is to be or was signed
      * @return data that is or was signed
      */
      std::vector<byte> tbs_data() const;

      /**
      * @return signature on tbs_data()
      */
      std::vector<byte> signature() const;

      /**
      * @return signature algorithm that was used to generate signature
      */
      AlgorithmIdentifier signature_algorithm() const;

      /**
      * @return hash algorithm that was used to generate signature
      */
      std::string hash_used_for_signature() const;

      /**
      * Create a signed X509 object.
      * @param signer the signer used to sign the object
      * @param rng the random number generator to use
      * @param alg_id the algorithm identifier of the signature scheme
      * @param tbs the tbs bits to be signed
      * @return signed X509 object
      */
      static std::vector<byte> make_signed(class PK_Signer* signer,
                                           RandomNumberGenerator& rng,
                                           const AlgorithmIdentifier& alg_id,
                                           const secure_vector<byte>& tbs);

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key& key) const;

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      *        the pointer will be deleted after use
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key* key) const;

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;

      /**
      * @return BER encoding of this
      */
      std::vector<byte> BER_encode() const;

      /**
      * @return PEM encoding of this
      */
      std::string PEM_encode() const;

      virtual ~X509_Object() {}
   protected:
      X509_Object(DataSource& src, const std::string& pem_labels);
      X509_Object(const std::string& file, const std::string& pem_labels);
      X509_Object(const std::vector<byte>& vec, const std::string& labels);

      void do_decode();
      X509_Object() {}
      AlgorithmIdentifier sig_algo;
      std::vector<byte> tbs_bits, sig;
   private:
      virtual void force_decode() = 0;
      void init(DataSource&, const std::string&);

      std::vector<std::string> PEM_labels_allowed;
      std::string PEM_label_pref;
   };

}


namespace Botan {

/**
* Distinguished Name
*/
class BOTAN_DLL X509_DN : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      std::multimap<OID, std::string> get_attributes() const;
      std::vector<std::string> get_attribute(const std::string&) const;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      void add_attribute(const OID&, const std::string&);

      static std::string deref_info_field(const std::string&);

      std::vector<byte> get_bits() const;

      X509_DN();
      X509_DN(const std::multimap<OID, std::string>&);
      X509_DN(const std::multimap<std::string, std::string>&);
   private:
      std::multimap<OID, ASN1_String> dn_info;
      std::vector<byte> dn_bits;
   };

bool BOTAN_DLL operator==(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator!=(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator<(const X509_DN&, const X509_DN&);

BOTAN_DLL std::ostream& operator<<(std::ostream& out, const X509_DN& dn);

}


namespace Botan {

/**
* Data Store
*/
class BOTAN_DLL Data_Store
   {
   public:
      /**
      * A search function
      */
      bool operator==(const Data_Store&) const;

      std::multimap<std::string, std::string> search_for(
         std::function<bool (std::string, std::string)> predicate) const;

      std::vector<std::string> get(const std::string&) const;

      std::string get1(const std::string& key) const;

      std::string get1(const std::string& key,
                       const std::string& default_value) const;

      std::vector<byte> get1_memvec(const std::string&) const;
      u32bit get1_u32bit(const std::string&, u32bit = 0) const;

      bool has_value(const std::string&) const;

      void add(const std::multimap<std::string, std::string>&);
      void add(const std::string&, const std::string&);
      void add(const std::string&, u32bit);
      void add(const std::string&, const secure_vector<byte>&);
      void add(const std::string&, const std::vector<byte>&);
   private:
      std::multimap<std::string, std::string> contents;
   };

}


namespace Botan {

/**
* X.509v3 Key Constraints.
*/
enum Key_Constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 32768,
   NON_REPUDIATION    = 16384,
   KEY_ENCIPHERMENT   = 8192,
   DATA_ENCIPHERMENT  = 4096,
   KEY_AGREEMENT      = 2048,
   KEY_CERT_SIGN      = 1024,
   CRL_SIGN           = 512,
   ENCIPHER_ONLY      = 256,
   DECIPHER_ONLY      = 128
};

class Public_Key;

/**
* Create the key constraints for a specific public key.
* @param pub_key the public key from which the basic set of
* constraints to be placed in the return value is derived
* @param limits additional limits that will be incorporated into the
* return value
* @return combination of key type specific constraints and
* additional limits
*/

BOTAN_DLL Key_Constraints find_constraints(const Public_Key& pub_key,
                                           Key_Constraints limits);

/**
* BER Decoding Function for key constraints
*/
namespace BER {

void BOTAN_DLL decode(BER_Decoder&, Key_Constraints&);

}

}


namespace Botan {

/**
* This class represents X.509 Certificate
*/
class BOTAN_DLL X509_Certificate : public X509_Object
   {
   public:
      /**
      * Get the public key associated with this certificate.
      * @return subject public key of this certificate
      */
      Public_Key* subject_public_key() const;

      /**
      * Get the public key associated with this certificate.
      * @return subject public key of this certificate
      */
      std::vector<byte> subject_public_key_bits() const;

      /**
      * Get the issuer certificate DN.
      * @return issuer DN of this certificate
      */
      X509_DN issuer_dn() const;

      /**
      * Get the subject certificate DN.
      * @return subject DN of this certificate
      */
      X509_DN subject_dn() const;

      /**
      * Get a value for a specific subject_info parameter name.
      * @param name the name of the paramter to look up. Possible names are
      * "X509.Certificate.version", "X509.Certificate.serial",
      * "X509.Certificate.start", "X509.Certificate.end",
      * "X509.Certificate.v2.key_id", "X509.Certificate.public_key",
      * "X509v3.BasicConstraints.path_constraint",
      * "X509v3.BasicConstraints.is_ca", "X509v3.ExtendedKeyUsage",
      * "X509v3.CertificatePolicies", "X509v3.SubjectKeyIdentifier" or
      * "X509.Certificate.serial".
      * @return value(s) of the specified parameter
      */
      std::vector<std::string> subject_info(const std::string& name) const;

      /**
      * Get a value for a specific subject_info parameter name.
      * @param name the name of the paramter to look up. Possible names are
      * "X509.Certificate.v2.key_id" or "X509v3.AuthorityKeyIdentifier".
      * @return value(s) of the specified parameter
      */
      std::vector<std::string> issuer_info(const std::string& name) const;

      /**
      * Raw subject DN
      */
      std::vector<byte> raw_issuer_dn() const;

      /**
      * Raw issuer DN
      */
      std::vector<byte> raw_subject_dn() const;

      /**
      * Get the notBefore of the certificate.
      * @return notBefore of the certificate
      */
      std::string start_time() const;

      /**
      * Get the notAfter of the certificate.
      * @return notAfter of the certificate
      */
      std::string end_time() const;

      /**
      * Get the X509 version of this certificate object.
      * @return X509 version
      */
      u32bit x509_version() const;

      /**
      * Get the serial number of this certificate.
      * @return certificates serial number
      */
      std::vector<byte> serial_number() const;

      /**
      * Get the DER encoded AuthorityKeyIdentifier of this certificate.
      * @return DER encoded AuthorityKeyIdentifier
      */
      std::vector<byte> authority_key_id() const;

      /**
      * Get the DER encoded SubjectKeyIdentifier of this certificate.
      * @return DER encoded SubjectKeyIdentifier
      */
      std::vector<byte> subject_key_id() const;

      /**
      * Check whether this certificate is self signed.
      * @return true if this certificate is self signed
      */
      bool is_self_signed() const { return self_signed; }

      /**
      * Check whether this certificate is a CA certificate.
      * @return true if this certificate is a CA certificate
      */
      bool is_CA_cert() const;

      bool allowed_usage(Key_Constraints usage) const;

      /**
      * Returns true if and only if name (referring to an extended key
      * constraint, eg "PKIX.ServerAuth") is included in the extended
      * key extension.
      */
      bool allowed_usage(const std::string& usage) const;

      /**
      * Get the path limit as defined in the BasicConstraints extension of
      * this certificate.
      * @return path limit
      */
      u32bit path_limit() const;

      /**
      * Get the key constraints as defined in the KeyUsage extension of this
      * certificate.
      * @return key constraints
      */
      Key_Constraints constraints() const;

      /**
      * Get the key constraints as defined in the ExtendedKeyUsage
      * extension of this
      * certificate.
      * @return key constraints
      */
      std::vector<std::string> ex_constraints() const;

      /**
      * Get the policies as defined in the CertificatePolicies extension
      * of this certificate.
      * @return certificate policies
      */
      std::vector<std::string> policies() const;

      /**
      * Return the listed address of an OCSP responder, or empty if not set
      */
      std::string ocsp_responder() const;

      /**
      * Return the CRL distribution point, or empty if not set
      */
      std::string crl_distribution_point() const;

      /**
      * @return a string describing the certificate
      */
      std::string to_string() const;

      /**
      * Return a fingerprint of the certificate
      */
      std::string fingerprint(const std::string& = "SHA-1") const;

      /**
      * Check if a certain DNS name matches up with the information in
      * the cert
      */
      bool matches_dns_name(const std::string& name) const;

      /**
      * Check to certificates for equality.
      * @return true both certificates are (binary) equal
      */
      bool operator==(const X509_Certificate& other) const;

      /**
      * Impose an arbitrary (but consistent) ordering
      * @return true if this is less than other by some unspecified criteria
      */
      bool operator<(const X509_Certificate& other) const;

      /**
      * Create a certificate from a data source providing the DER or
      * PEM encoded certificate.
      * @param source the data source
      */
      X509_Certificate(DataSource& source);

      /**
      * Create a certificate from a file containing the DER or PEM
      * encoded certificate.
      * @param filename the name of the certificate file
      */
      X509_Certificate(const std::string& filename);

      X509_Certificate(const std::vector<byte>& in);

   private:
      void force_decode();
      friend class X509_CA;
      friend class BER_Decoder;

      X509_Certificate() {}

      Data_Store subject, issuer;
      bool self_signed;
   };

/**
* Check two certificates for inequality
* @return true if the arguments represent different certificates,
* false if they are binary identical
*/
BOTAN_DLL bool operator!=(const X509_Certificate&, const X509_Certificate&);

/*
* Data Store Extraction Operations
*/
BOTAN_DLL X509_DN create_dn(const Data_Store&);
BOTAN_DLL AlternativeName create_alt_name(const Data_Store&);

}


namespace Botan {

/**
* X.509v2 CRL Reason Code.
*/
enum CRL_Code {
   UNSPECIFIED            = 0,
   KEY_COMPROMISE         = 1,
   CA_COMPROMISE          = 2,
   AFFILIATION_CHANGED    = 3,
   SUPERSEDED             = 4,
   CESSATION_OF_OPERATION = 5,
   CERTIFICATE_HOLD       = 6,
   REMOVE_FROM_CRL        = 8,
   PRIVLEDGE_WITHDRAWN    = 9,
   AA_COMPROMISE          = 10,

   DELETE_CRL_ENTRY       = 0xFF00,
   OCSP_GOOD              = 0xFF01,
   OCSP_UNKNOWN           = 0xFF02
};

/**
* This class represents CRL entries
*/
class BOTAN_DLL CRL_Entry : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      /**
      * Get the serial number of the certificate associated with this entry.
      * @return certificate's serial number
      */
      std::vector<byte> serial_number() const { return serial; }

      /**
      * Get the revocation date of the certificate associated with this entry
      * @return certificate's revocation date
      */
      X509_Time expire_time() const { return time; }

      /**
      * Get the entries reason code
      * @return reason code
      */
      CRL_Code reason_code() const { return reason; }

      /**
      * Construct an empty CRL entry.
      */
      CRL_Entry(bool throw_on_unknown_critical_extension = false);

      /**
      * Construct an CRL entry.
      * @param cert the certificate to revoke
      * @param reason the reason code to set in the entry
      */
      CRL_Entry(const X509_Certificate& cert,
                CRL_Code reason = UNSPECIFIED);

   private:
      bool throw_on_unknown_critical;
      std::vector<byte> serial;
      X509_Time time;
      CRL_Code reason;
   };

/**
* Test two CRL entries for equality in all fields.
*/
BOTAN_DLL bool operator==(const CRL_Entry&, const CRL_Entry&);

/**
* Test two CRL entries for inequality in at least one field.
*/
BOTAN_DLL bool operator!=(const CRL_Entry&, const CRL_Entry&);

}


namespace Botan {

class X509_Certificate;

/**
* This class represents X.509 Certificate Revocation Lists (CRLs).
*/
class BOTAN_DLL X509_CRL : public X509_Object
   {
   public:
      /**
      * This class represents CRL related errors.
      */
      struct BOTAN_DLL X509_CRL_Error : public Exception
         {
         X509_CRL_Error(const std::string& error) :
            Exception("X509_CRL: " + error) {}
         };

      /**
      * Check if this particular certificate is listed in the CRL
      */
      bool is_revoked(const X509_Certificate& cert) const;

      /**
      * Get the entries of this CRL in the form of a vector.
      * @return vector containing the entries of this CRL.
      */
      std::vector<CRL_Entry> get_revoked() const;

      /**
      * Get the issuer DN of this CRL.
      * @return CRLs issuer DN
      */
      X509_DN issuer_dn() const;

      /**
      * Get the AuthorityKeyIdentifier of this CRL.
      * @return this CRLs AuthorityKeyIdentifier
      */
      std::vector<byte> authority_key_id() const;

      /**
      * Get the serial number of this CRL.
      * @return CRLs serial number
      */
      u32bit crl_number() const;

      /**
      * Get the CRL's thisUpdate value.
      * @return CRLs thisUpdate
      */
      X509_Time this_update() const;

      /**
      * Get the CRL's nextUpdate value.
      * @return CRLs nextdUpdate
      */
      X509_Time next_update() const;

      /**
      * Construct a CRL from a data source.
      * @param source the data source providing the DER or PEM encoded CRL.
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(DataSource& source, bool throw_on_unknown_critical = false);

      /**
      * Construct a CRL from a file containing the DER or PEM encoded CRL.
      * @param filename the name of the CRL file
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::string& filename,
               bool throw_on_unknown_critical = false);

      /**
      * Construct a CRL from a binary vector
      * @param vec the binary (DER) representation of the CRL
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::vector<byte>& vec,
               bool throw_on_unknown_critical = false);

   private:
      void force_decode();

      bool throw_on_unknown_critical;
      std::vector<CRL_Entry> revoked;
      Data_Store info;
   };

}


namespace Botan {

/**
* Certificate Store Interface
*/
class BOTAN_DLL Certificate_Store
   {
   public:
      virtual ~Certificate_Store() {}

      /**
      * Subject DN and (optionally) key identifier
      */
      virtual const X509_Certificate*
         find_cert(const X509_DN& subject_dn, const std::vector<byte>& key_id) const = 0;

      virtual const X509_CRL* find_crl_for(const X509_Certificate& subject) const;

      bool certificate_known(const X509_Certificate& cert) const
         {
         return find_cert(cert.subject_dn(), cert.subject_key_id()) != nullptr;
         }

      // remove this (used by TLS::Server)
      virtual std::vector<X509_DN> all_subjects() const = 0;
   };

/**
* In Memory Certificate Store
*/
class BOTAN_DLL Certificate_Store_In_Memory : public Certificate_Store
   {
   public:
      /**
      * Attempt to parse all files in dir (including subdirectories)
      * as certificates. Ignores errors.
      */
      Certificate_Store_In_Memory(const std::string& dir);

      Certificate_Store_In_Memory(const X509_Certificate& cert);

      Certificate_Store_In_Memory() {}

      void add_certificate(const X509_Certificate& cert);

      void add_crl(const X509_CRL& crl);

      std::vector<X509_DN> all_subjects() const override;

      const X509_Certificate* find_cert(
         const X509_DN& subject_dn,
         const std::vector<byte>& key_id) const override;

      const X509_CRL* find_crl_for(const X509_Certificate& subject) const override;
   private:
      // TODO: Add indexing on the DN and key id to avoid linear search
      std::vector<X509_Certificate> m_certs;
      std::vector<X509_CRL> m_crls;
   };

class BOTAN_DLL Certificate_Store_Overlay : public Certificate_Store
   {
   public:
      Certificate_Store_Overlay(const std::vector<X509_Certificate>& certs) :
         m_certs(certs) {}

      std::vector<X509_DN> all_subjects() const override;

      const X509_Certificate* find_cert(
         const X509_DN& subject_dn,
         const std::vector<byte>& key_id) const override;
   private:
      const std::vector<X509_Certificate>& m_certs;
   };

}


namespace Botan {

/**
* CFB Mode
*/
class BOTAN_DLL CFB_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t output_length(size_t input_length) const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      CFB_Mode(BlockCipher* cipher, size_t feedback_bits);

      const BlockCipher& cipher() const { return *m_cipher; }

      size_t feedback() const { return m_feedback_bytes; }

      secure_vector<byte>& shift_register() { return m_shift_register; }

      secure_vector<byte>& keystream_buf() { return m_keystream_buf; }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;
      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_shift_register;
      secure_vector<byte> m_keystream_buf;
      size_t m_feedback_bytes;
   };

/**
* CFB Encryption
*/
class BOTAN_DLL CFB_Encryption : public CFB_Mode
   {
   public:
      CFB_Encryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* CFB Decryption
*/
class BOTAN_DLL CFB_Decryption : public CFB_Mode
   {
   public:
      CFB_Decryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* DJB's ChaCha (http://cr.yp.to/chacha.html)
*/
class BOTAN_DLL ChaCha : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len == 8 || iv_len == 12); }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear();
      std::string name() const { return "ChaCha"; }

      StreamCipher* clone() const { return new ChaCha; }
   protected:
      virtual void chacha(byte output[64], const u32bit input[16]);
   private:
      void key_schedule(const byte key[], size_t key_len);

      secure_vector<u32bit> m_state;
      secure_vector<byte> m_buffer;
      size_t m_position = 0;
   };

}


namespace Botan {

/**
* Base class
* See draft-irtf-cfrg-chacha20-poly1305-03 for specification
* If a nonce of 64 bits is used the older version described in
* draft-agl-tls-chacha20poly1305-04 is used instead.
*/
class BOTAN_DLL ChaCha20Poly1305_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override { return "ChaCha20Poly1305"; }

      size_t update_granularity() const override { return 64; }

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(32); }

      bool valid_nonce_length(size_t n) const override;

      size_t tag_size() const override { return 16; }

      void clear() override;
   protected:
      std::unique_ptr<StreamCipher> m_chacha;
      std::unique_ptr<MessageAuthenticationCode> m_poly1305;

      ChaCha20Poly1305_Mode();

      secure_vector<byte> m_ad;
      size_t m_nonce_len = 0;
      size_t m_ctext_len = 0;

      bool cfrg_version() const { return m_nonce_len == 12; }
      void update_len(size_t len);
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* ChaCha20Poly1305 Encryption
*/
class BOTAN_DLL ChaCha20Poly1305_Encryption : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* ChaCha20Poly1305 Decryption
*/
class BOTAN_DLL ChaCha20Poly1305_Decryption : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* The different charsets (nominally) supported by Botan.
*/
enum Character_Set {
   LOCAL_CHARSET,
   UCS2_CHARSET,
   UTF8_CHARSET,
   LATIN1_CHARSET
};

namespace Charset {

/*
* Character Set Handling
*/
std::string BOTAN_DLL transcode(const std::string& str,
                                Character_Set to,
                                Character_Set from);

bool BOTAN_DLL is_digit(char c);
bool BOTAN_DLL is_space(char c);
bool BOTAN_DLL caseless_cmp(char x, char y);

byte BOTAN_DLL char2digit(char c);
char BOTAN_DLL digit2char(byte b);

}

}


namespace Botan {

/**
* CMAC, also known as OMAC1
*/
class BOTAN_DLL CMAC : public MessageAuthenticationCode
   {
   public:
      std::string name() const;
      size_t output_length() const { return m_cipher->block_size(); }
      MessageAuthenticationCode* clone() const;

      void clear();

      Key_Length_Specification key_spec() const
         {
         return m_cipher->key_spec();
         }

      /**
      * CMAC's polynomial doubling operation
      * @param in the input
      * @param polynomial the byte value of the polynomial
      */
      static secure_vector<byte> poly_double(const secure_vector<byte>& in);

      /**
      * @param cipher the underlying block cipher to use
      */
      CMAC(BlockCipher* cipher);

      static CMAC* make(const Spec& spec);

      CMAC(const CMAC&) = delete;
      CMAC& operator=(const CMAC&) = delete;
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_buffer, m_state, m_B, m_P;
      size_t m_position;
   };

}


namespace Botan {

namespace gf2m_small_m {

typedef u16bit gf2m;

class Gf2m_Field
   {
   public:
      Gf2m_Field(size_t extdeg);

      gf2m gf_mul(gf2m x, gf2m y)
         {
         return ((x) ? gf_mul_fast(x, y) : 0);
         }

      gf2m gf_square(gf2m x)
         {
         return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] << 1)] : 0);
         }

      gf2m square_rr(gf2m x)
         {
         return _gf_modq_1(x << 1);
         }

      // naming convention of GF(2^m) field operations:
      // l logarithmic, unreduced
      // r logarithmic, reduced
      // n normal, non-zero
      // z normal, might be zero
      //
      inline gf2m gf_mul_lll(gf2m a, gf2m b);
      inline gf2m gf_mul_rrr(gf2m a, gf2m b);
      inline gf2m gf_mul_nrr(gf2m a, gf2m b);
      inline gf2m gf_mul_rrn(gf2m a, gf2m y);
      inline gf2m gf_mul_lnn(gf2m x, gf2m y);
      inline gf2m gf_mul_rnn(gf2m x, gf2m y);
      inline gf2m gf_mul_nrn(gf2m a, gf2m y);
      inline gf2m gf_mul_rnr(gf2m y, gf2m a);
      inline gf2m gf_mul_zrz(gf2m a, gf2m y);
      inline gf2m gf_mul_zzr(gf2m a, gf2m y);
      inline gf2m gf_mul_nnr(gf2m y, gf2m a);
      inline gf2m gf_sqrt(gf2m x) ;
      gf2m gf_div(gf2m x, gf2m y);
      inline gf2m gf_div_rnn(gf2m x, gf2m y);
      inline gf2m gf_div_rnr(gf2m x, gf2m b);
      inline gf2m gf_div_nrr(gf2m a, gf2m b);
      inline gf2m gf_div_zzr(gf2m x, gf2m b);
      inline gf2m gf_inv(gf2m x);
      inline gf2m gf_inv_rn(gf2m x);
      inline gf2m gf_square_ln(gf2m x);
      inline gf2m gf_square_rr(gf2m a) ;
      inline gf2m gf_l_from_n(gf2m x);

      inline gf2m gf_mul_fast(gf2m a, gf2m b);

      gf2m gf_exp(gf2m i)
         {
         return m_gf_exp_table[i]; /* alpha^i */
         }

      gf2m gf_log(gf2m i)
         {
         return m_gf_log_table[i]; /* return i when x=alpha^i */
         }

      inline gf2m gf_ord() const
         {
         return m_gf_multiplicative_order;
         }

      inline gf2m get_extension_degree() const
         {
         return m_gf_extension_degree;
         }

      inline gf2m get_cardinality() const
         {
         return m_gf_cardinality;
         }

      gf2m gf_pow(gf2m x, int i) ;

   private:
      gf2m m_gf_extension_degree, m_gf_cardinality, m_gf_multiplicative_order;
      std::vector<gf2m> m_gf_log_table;
      std::vector<gf2m> m_gf_exp_table;

      inline gf2m _gf_modq_1(s32bit d);
      void init_log();
      void init_exp();
   };

gf2m Gf2m_Field::_gf_modq_1(s32bit d)
   {
   return  (((d) & gf_ord()) + ((d) >> m_gf_extension_degree));
   }

gf2m Gf2m_Field::gf_mul_fast(gf2m x, gf2m y)
   {
   return ((y) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] + m_gf_log_table[y])] : 0);
   }

gf2m Gf2m_Field::gf_mul_lll(gf2m a, gf2m b)
   {
   return  (a + b);
   }

gf2m Gf2m_Field::gf_mul_rrr(gf2m a, gf2m b)
   {
   return (_gf_modq_1(gf_mul_lll(a, b)));
   }

gf2m Gf2m_Field::gf_mul_nrr(gf2m a, gf2m b)
   {
   return (gf_exp(gf_mul_rrr(a, b)));
   }

gf2m Gf2m_Field::gf_mul_rrn(gf2m a, gf2m y)
   {
   return _gf_modq_1(gf_mul_lll(a, gf_log(y)));
   }

gf2m Gf2m_Field::gf_mul_rnr(gf2m y, gf2m a)
   {
   return gf_mul_rrn(a, y);
   }

gf2m Gf2m_Field::gf_mul_lnn(gf2m x, gf2m y)
   {
   return (m_gf_log_table[x] + m_gf_log_table[y]);
   }
gf2m Gf2m_Field::gf_mul_rnn(gf2m x, gf2m y)
   {
   return _gf_modq_1(gf_mul_lnn(x, y));
   }

gf2m Gf2m_Field::gf_mul_nrn(gf2m a, gf2m y)
   {
   return m_gf_exp_table[_gf_modq_1((a) + m_gf_log_table[y])];
   }

/**
* zero operand allowed
*/
gf2m Gf2m_Field::gf_mul_zrz(gf2m a, gf2m y)
   {
   return ( (y == 0) ? 0 : gf_mul_nrn(a, y) );
   }

gf2m Gf2m_Field::gf_mul_zzr(gf2m a, gf2m y)
   {
   return gf_mul_zrz(y, a);
   }
/**
* non-zero operand
*/
gf2m Gf2m_Field::gf_mul_nnr(gf2m y, gf2m a)
   {
   return gf_mul_nrn( a, y);
   }

gf2m Gf2m_Field::gf_sqrt(gf2m x)
   {
   return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] << (m_gf_extension_degree-1))] : 0);
   }

gf2m Gf2m_Field::gf_div_rnn(gf2m x, gf2m y)
   {
   return _gf_modq_1(m_gf_log_table[x] - m_gf_log_table[y]);
   }
gf2m Gf2m_Field::gf_div_rnr(gf2m x, gf2m b)
   {
   return _gf_modq_1(m_gf_log_table[x] - b);
   }
gf2m Gf2m_Field::gf_div_nrr(gf2m a, gf2m b)
   {
   return m_gf_exp_table[_gf_modq_1(a - b)];
   }

gf2m Gf2m_Field::gf_div_zzr(gf2m x, gf2m b)
   {
   return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] - b)] : 0);
   }

gf2m Gf2m_Field::gf_inv(gf2m x)
   {
   return m_gf_exp_table[gf_ord() - m_gf_log_table[x]];
   }
gf2m Gf2m_Field::gf_inv_rn(gf2m x)
   {
   return (gf_ord() - m_gf_log_table[x]);
   }

gf2m Gf2m_Field::gf_square_ln(gf2m x)
   {
   return m_gf_log_table[x] << 1;
   }

gf2m Gf2m_Field::gf_square_rr(gf2m a)
   {
   return a << 1;
   }

gf2m Gf2m_Field::gf_l_from_n(gf2m x)
   {
   return m_gf_log_table[x];
   }

u32bit encode_gf2m(gf2m to_enc, byte* mem);

gf2m decode_gf2m(const byte* mem);

}

}


namespace Botan {

/**
* Expand an input to a bit mask depending on it being being zero or non-zero
* @ param tst the input
* @return the mask 0xFFFF if tst is non-zero and 0 otherwise
*/
template<typename T>
u16bit expand_mask_16bit(T tst)
   {
   const u16bit result = (tst != 0);
   return ~(result - 1);
   }

inline gf2m_small_m::gf2m gray_to_lex(gf2m_small_m::gf2m gray)
   {
   gf2m_small_m::gf2m result = gray ^ (gray>>8);
   result ^= (result >> 4);
   result ^= (result >> 2);
   result ^= (result >> 1);
   return result;
   }

inline gf2m_small_m::gf2m lex_to_gray(gf2m_small_m::gf2m lex)
   {
   return (lex>>1) ^ lex;
   }

inline u32bit bit_size_to_byte_size(u32bit bit_size)
   {
   return (bit_size - 1) / 8 + 1;
   }

inline u32bit bit_size_to_32bit_size(u32bit bit_size)
   {
   return (bit_size - 1) / 32 + 1;
   }

}


namespace Botan {

/**
* Combines two hash functions using a Feistel scheme. Described in
* "On the Security of Hash Function Combiners", Anja Lehmann
*/
class BOTAN_DLL Comb4P : public HashFunction
   {
   public:
      /**
      * @param h1 the first hash
      * @param h2 the second hash
      */
      Comb4P(HashFunction* h1, HashFunction* h2);

      size_t hash_block_size() const;

      size_t output_length() const
         {
         return m_hash1->output_length() + m_hash2->output_length();
         }

      static Comb4P* make(const Spec& spec);
      
      HashFunction* clone() const
         {
         return new Comb4P(m_hash1->clone(), m_hash2->clone());
         }

      std::string name() const
         {
         return "Comb4P(" + m_hash1->name() + "," + m_hash2->name() + ")";
         }

      void clear();
   private:
      void add_data(const byte input[], size_t length);
      void final_result(byte out[]);

      std::unique_ptr<HashFunction> m_hash1, m_hash2;
   };

}


namespace Botan {

class Transform;
class Compressor_Transform;

/**
* Filter interface for compression/decompression
*/
class BOTAN_DLL Compression_Decompression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const byte input[], size_t input_length) override;
      void end_msg() override;

      std::string name() const override;

   protected:
      Compression_Decompression_Filter(Transform* t);

      void flush();
   private:
      std::unique_ptr<Compressor_Transform> m_transform;
      secure_vector<byte> m_buffer;
   };

class BOTAN_DLL Compression_Filter : public Compression_Decompression_Filter
   {
   public:
      Compression_Filter(const std::string& type, size_t level); // compression

      using Compression_Decompression_Filter::flush;
   };

class Decompression_Filter : public Compression_Decompression_Filter
   {
   public:
      Decompression_Filter(const std::string& type);
   };

}


namespace Botan {

class BOTAN_DLL Compressor_Transform : public Transform
   {
   public:
      size_t update_granularity() const override { return 1; }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return nonce_len == 0; }

      virtual void flush(secure_vector<byte>& buf, size_t offset = 0) { update(buf, offset); }

      size_t output_length(size_t) const override
         {
         throw std::runtime_error(name() + " output length indeterminate");
         }
   };

BOTAN_DLL Transform* make_compressor(const std::string& type, size_t level);
BOTAN_DLL Transform* make_decompressor(const std::string& type);

class Compression_Stream
   {
   public:
      virtual ~Compression_Stream() {}

      virtual void next_in(byte* b, size_t len) = 0;

      virtual void next_out(byte* b, size_t len) = 0;

      virtual size_t avail_in() const = 0;

      virtual size_t avail_out() const = 0;

      virtual u32bit run_flag() const = 0;
      virtual u32bit flush_flag() const = 0;
      virtual u32bit finish_flag() const = 0;

      virtual bool run(u32bit flags) = 0;
   };

class BOTAN_DLL Stream_Compression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void flush(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;
   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

class BOTAN_DLL Stream_Decompression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;

   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

}


namespace Botan {

/**
* A class handling runtime CPU feature detection
*/
class BOTAN_DLL CPUID
   {
   public:
      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      /**
      * Return a best guess of the cache line size
      */
      static size_t cache_line_size() { initialize(); return g_cache_line_size; }

      /**
      * Check if the processor supports AltiVec/VMX
      */
      static bool has_altivec() { initialize(); return g_altivec_capable; }

      /**
      * Check if the processor supports RDTSC
      */
      static bool has_rdtsc()
         { return x86_processor_flags_has(CPUID_RDTSC_BIT); }

      /**
      * Check if the processor supports SSE2
      */
      static bool has_sse2()
         { return x86_processor_flags_has(CPUID_SSE2_BIT); }

      /**
      * Check if the processor supports SSSE3
      */
      static bool has_ssse3()
         { return x86_processor_flags_has(CPUID_SSSE3_BIT); }

      /**
      * Check if the processor supports SSE4.1
      */
      static bool has_sse41()
         { return x86_processor_flags_has(CPUID_SSE41_BIT); }

      /**
      * Check if the processor supports SSE4.2
      */
      static bool has_sse42()
         { return x86_processor_flags_has(CPUID_SSE42_BIT); }

      /**
      * Check if the processor supports AVX2
      */
      static bool has_avx2()
         { return x86_processor_flags_has(CPUID_AVX2_BIT); }

      /**
      * Check if the processor supports AVX-512F
      */
      static bool has_avx512f()
         { return x86_processor_flags_has(CPUID_AVX512F_BIT); }

      /**
      * Check if the processor supports BMI2
      */
      static bool has_bmi2()
         { return x86_processor_flags_has(CPUID_BMI2_BIT); }

      /**
      * Check if the processor supports AES-NI
      */
      static bool has_aes_ni()
         { return x86_processor_flags_has(CPUID_AESNI_BIT); }

      /**
      * Check if the processor supports CLMUL
      */
      static bool has_clmul()
         { return x86_processor_flags_has(CPUID_CLMUL_BIT); }

      /**
      * Check if the processor supports Intel SHA extension
      */
      static bool has_intel_sha()
         { return x86_processor_flags_has(CPUID_SHA_BIT); }

      /**
      * Check if the processor supports ADX extension
      */
      static bool has_adx()
         { return x86_processor_flags_has(CPUID_ADX_BIT); }

      /**
      * Check if the processor supports RDRAND
      */
      static bool has_rdrand()
         { return x86_processor_flags_has(CPUID_RDRAND_BIT); }

      /**
      * Check if the processor supports RDSEED
      */
      static bool has_rdseed()
         { return x86_processor_flags_has(CPUID_RDSEED_BIT); }

      static void print(std::ostream& o);
   private:
      enum CPUID_bits {
         CPUID_RDTSC_BIT = 4,
         CPUID_SSE2_BIT = 26,
         CPUID_CLMUL_BIT = 33,
         CPUID_SSSE3_BIT = 41,
         CPUID_SSE41_BIT = 51,
         CPUID_SSE42_BIT = 52,
         CPUID_AESNI_BIT = 57,
         CPUID_RDRAND_BIT = 62,

         CPUID_AVX2_BIT = 64+5,
         CPUID_BMI2_BIT = 64+8,
         CPUID_AVX512F_BIT = 64+16,
         CPUID_RDSEED_BIT = 64+18,
         CPUID_ADX_BIT = 64+19,
         CPUID_SHA_BIT = 64+29,
      };

      static bool x86_processor_flags_has(u64bit bit)
         {
         if(!g_initialized)
            initialize();
         return ((g_x86_processor_flags[bit/64] >> (bit % 64)) & 1);
         }

      static bool g_initialized;
      static u64bit g_x86_processor_flags[2];
      static size_t g_cache_line_size;
      static bool g_altivec_capable;
   };

}


namespace Botan {

/**
* 24-bit cyclic redundancy check
*/
class BOTAN_DLL CRC24 : public HashFunction
   {
   public:
      std::string name() const { return "CRC24"; }
      size_t output_length() const { return 3; }
      HashFunction* clone() const { return new CRC24; }

      void clear() { crc = 0xB704CE; }

      CRC24() { clear(); }
      ~CRC24() { clear(); }
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      u32bit crc;
   };

}


namespace Botan {

/**
* 32-bit cyclic redundancy check
*/
class BOTAN_DLL CRC32 : public HashFunction
   {
   public:
      std::string name() const { return "CRC32"; }
      size_t output_length() const { return 4; }
      HashFunction* clone() const { return new CRC32; }

      void clear() { crc = 0xFFFFFFFF; }

      CRC32() { clear(); }
      ~CRC32() { clear(); }
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      u32bit crc;
   };

}


namespace Botan {

class BigInt;

/**
* Interface for a credentials manager.
*
* A type is a fairly static value that represents the general nature
* of the transaction occuring. Currently used values are "tls-client"
* and "tls-server". Context represents a hostname, email address,
* username, or other identifier.
*/
class BOTAN_DLL Credentials_Manager
   {
   public:
      virtual ~Credentials_Manager() {}

      /**
      * Return a list of the certificates of CAs that we trust in this
      * type/context.
      *
      * @param type specifies the type of operation occuring
      *
      * @param context specifies a context relative to type. For instance
      *        for type "tls-client", context specifies the servers name.
      */
      virtual std::vector<Certificate_Store*> trusted_certificate_authorities(
         const std::string& type,
         const std::string& context);

      /**
      * Check the certificate chain is valid up to a trusted root, and
      * optionally (if hostname != "") that the hostname given is
      * consistent with the leaf certificate.
      *
      * This function should throw an exception derived from
      * std::exception with an informative what() result if the
      * certificate chain cannot be verified.

      * @param type specifies the type of operation occuring
      * @param hostname specifies the purported hostname
      * @param cert_chain specifies a certificate chain leading to a
      *        trusted root CA certificate.
      */
      virtual void verify_certificate_chain(
         const std::string& type,
         const std::string& hostname,
         const std::vector<X509_Certificate>& cert_chain);

      /**
      * Return a cert chain we can use, ordered from leaf to root,
      * or else an empty vector.
      *
      * It is assumed that the caller can get the private key of the
      * leaf with private_key_for
      *
      * @param cert_key_types specifies the key types desired ("RSA",
      *                       "DSA", "ECDSA", etc), or empty if there
      *                       is no preference by the caller.
      *
      * @param type specifies the type of operation occuring
      *
      * @param context specifies a context relative to type.
      */
      virtual std::vector<X509_Certificate> cert_chain(
         const std::vector<std::string>& cert_key_types,
         const std::string& type,
         const std::string& context);

      /**
      * Return a cert chain we can use, ordered from leaf to root,
      * or else an empty vector.
      *
      * It is assumed that the caller can get the private key of the
      * leaf with private_key_for
      *
      * @param cert_key_type specifies the type of key requested
      *                      ("RSA", "DSA", "ECDSA", etc)
      *
      * @param type specifies the type of operation occuring
      *
      * @param context specifies a context relative to type.
      */
      std::vector<X509_Certificate> cert_chain_single_type(
         const std::string& cert_key_type,
         const std::string& type,
         const std::string& context);

      /**
      * @return private key associated with this certificate if we should
      *         use it with this context. cert was returned by cert_chain
      * @note this object should retain ownership of the returned key;
      *       it should not be deleted by the caller.
      */
      virtual Private_Key* private_key_for(const X509_Certificate& cert,
                                           const std::string& type,
                                           const std::string& context);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @return true if we should attempt SRP authentication
      */
      virtual bool attempt_srp(const std::string& type,
                               const std::string& context);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @return identifier for client-side SRP auth, if available
                for this type/context. Should return empty string
                if password auth not desired/available.
      */
      virtual std::string srp_identifier(const std::string& type,
                                         const std::string& context);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @param identifier specifies what identifier we want the
      *        password for. This will be a value previously returned
      *        by srp_identifier.
      * @return password for client-side SRP auth, if available
                for this identifier/type/context.
      */
      virtual std::string srp_password(const std::string& type,
                                       const std::string& context,
                                       const std::string& identifier);

      /**
      * Retrieve SRP verifier parameters
      */
      virtual bool srp_verifier(const std::string& type,
                                const std::string& context,
                                const std::string& identifier,
                                std::string& group_name,
                                BigInt& verifier,
                                std::vector<byte>& salt,
                                bool generate_fake_on_unknown);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @return the PSK identity hint for this type/context
      */
      virtual std::string psk_identity_hint(const std::string& type,
                                            const std::string& context);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @param identity_hint was passed by the server (but may be empty)
      * @return the PSK identity we want to use
      */
      virtual std::string psk_identity(const std::string& type,
                                       const std::string& context,
                                       const std::string& identity_hint);

      /**
      * @param type specifies the type of operation occuring
      * @param context specifies a context relative to type.
      * @param identity is a PSK identity previously returned by
               psk_identity for the same type and context.
      * @return the PSK used for identity, or throw an exception if no
      * key exists
      */
      virtual SymmetricKey psk(const std::string& type,
                               const std::string& context,
                               const std::string& identity);
   };

}


namespace Botan {

/**
* This namespace holds various high-level crypto functions
*/
namespace CryptoBox {

/**
* Encrypt a message using a passphrase
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
* @param rng a ref to a random number generator, such as AutoSeeded_RNG
*/
BOTAN_DLL std::string encrypt(const byte input[], size_t input_len,
                              const std::string& passphrase,
                              RandomNumberGenerator& rng);


/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const byte input[], size_t input_len,
                              const std::string& passphrase);

/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const std::string& input,
                              const std::string& passphrase);

}

}


namespace Botan {

/**
* CTR-BE (Counter mode, big-endian)
*/
class BOTAN_DLL CTR_BE : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const
         {
         return m_cipher->key_spec();
         }

      std::string name() const;

      CTR_BE* clone() const
         { return new CTR_BE(m_cipher->clone()); }

      void clear();

      static CTR_BE* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      CTR_BE(BlockCipher* cipher);
   private:
      void key_schedule(const byte key[], size_t key_len);
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_counter, m_pad;
      size_t m_pad_pos;
   };

}


namespace Botan {

class BOTAN_DLL Curve25519_PublicKey : public virtual Public_Key
   {
   public:
      std::string algo_name() const override { return "Curve25519"; }

      size_t estimated_strength() const override { return 128; }

      size_t max_input_bits() const { return 256; }

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<byte> x509_subject_public_key() const override;

      std::vector<byte> public_value() const { return unlock(m_public); }

      Curve25519_PublicKey(const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits);

      Curve25519_PublicKey(const secure_vector<byte>& pub) : m_public(pub) {}
   protected:
      Curve25519_PublicKey() {}
      secure_vector<byte> m_public;
   };

class BOTAN_DLL Curve25519_PrivateKey : public Curve25519_PublicKey,
                                        public virtual Private_Key,
                                        public virtual PK_Key_Agreement_Key
   {
   public:
      Curve25519_PrivateKey(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits,
                            RandomNumberGenerator& rng);

      Curve25519_PrivateKey(RandomNumberGenerator& rng);

      Curve25519_PrivateKey(const secure_vector<byte>& secret_key);

      std::vector<byte> public_value() const override { return Curve25519_PublicKey::public_value(); }

      secure_vector<byte> agree(const byte w[], size_t w_len) const;

      const secure_vector<byte>& get_x() const { return m_private; }

      secure_vector<byte> pkcs8_private_key() const override;

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;
   private:
      secure_vector<byte> m_private;
   };

/*
* The types above are just wrappers for curve25519_donna, plus defining
* encodings for public and private keys.
*/
int BOTAN_DLL curve25519_donna(uint8_t mypublic[32],
                               const uint8_t secret[32],
                               const uint8_t basepoint[32]);

}


namespace Botan {

class CurveGFp_Repr
   {
   public:
      virtual ~CurveGFp_Repr() {}

      virtual const BigInt& get_p() const = 0;
      virtual const BigInt& get_a() const = 0;
      virtual const BigInt& get_b() const = 0;

      virtual size_t get_p_words() const = 0;

      /*
      * Returns to_curve_rep(get_a())
      */
      virtual const BigInt& get_a_rep() const = 0;

      /*
      * Returns to_curve_rep(get_b())
      */
      virtual const BigInt& get_b_rep() const = 0;

      virtual void to_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void from_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void curve_mul(BigInt& z, const BigInt& x, const BigInt& y,
                             secure_vector<word>& ws) const = 0;

      virtual void curve_sqr(BigInt& z, const BigInt& x,
                             secure_vector<word>& ws) const = 0;

      virtual void normalize(BigInt& x,
                             secure_vector<word>& ws,
                             size_t bound) const;
   };

/**
* This class represents an elliptic curve over GF(p)
*/
class BOTAN_DLL CurveGFp
   {
   public:

      /**
      * Create an uninitialized CurveGFp
      */
      CurveGFp() {}

      /**
      * Construct the elliptic curve E: y^2 = x^3 + ax + b over GF(p)
      * @param p prime number of the field
      * @param a first coefficient
      * @param b second coefficient
      */
      CurveGFp(const BigInt& p, const BigInt& a, const BigInt& b) :
         m_repr(choose_repr(p, a, b))
         {
         }

      CurveGFp(const CurveGFp&) = default;

      CurveGFp& operator=(const CurveGFp&) = default;

      /**
      * @return curve coefficient a
      */
      const BigInt& get_a() const { return m_repr->get_a(); }

      /**
      * @return curve coefficient b
      */
      const BigInt& get_b() const { return m_repr->get_b(); }

      /**
      * Get prime modulus of the field of the curve
      * @return prime modulus of the field of the curve
      */
      const BigInt& get_p() const { return m_repr->get_p(); }

      const BigInt& get_a_rep() const { return m_repr->get_a_rep(); }

      const BigInt& get_b_rep() const { return m_repr->get_b_rep(); }

      void to_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->to_curve_rep(x, ws);
         }

      void from_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->from_curve_rep(x, ws);
         }

      BigInt from_rep(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt xt(x);
         m_repr->from_curve_rep(xt, ws);
         return xt;
         }

      // TODO: from_rep taking && ref

      void mul(BigInt& z, const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         m_repr->curve_mul(z, x, y, ws);
         }

      BigInt mul(const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_mul(z, x, y, ws);
         return z;
         }

      void sqr(BigInt& z, const BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->curve_sqr(z, x, ws);
         }

      BigInt sqr(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_sqr(z, x, ws);
         return z;
         }

      /**
      * Adjust x to be in [0,p)
      * @param bound if greater than zero, assume that no more than bound
      *        additions or subtractions are required to move x into range.
      */
      void normalize(BigInt& x, secure_vector<word>& ws, size_t bound = 0) const
         {
         m_repr->normalize(x, ws, bound);
         }

      void swap(CurveGFp& other)
         {
         std::swap(m_repr, other.m_repr);
         }

   private:
      static std::shared_ptr<CurveGFp_Repr>
         choose_repr(const BigInt& p, const BigInt& a, const BigInt& b);

      std::shared_ptr<CurveGFp_Repr> m_repr;
   };

/**
* Equality operator
* @param lhs a curve
* @param rhs a curve
* @return true iff lhs is the same as rhs
*/
inline bool operator==(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return (lhs.get_p() == rhs.get_p()) &&
          (lhs.get_a() == rhs.get_a()) &&
          (lhs.get_b() == rhs.get_b());
   }

inline bool operator!=(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return !(lhs == rhs);
   }

}

namespace std {

template<> inline
void swap<Botan::CurveGFp>(Botan::CurveGFp& curve1,
                           Botan::CurveGFp& curve2) noexcept
   {
   curve1.swap(curve2);
   }

} // namespace std


namespace Botan {

/**
* This class represents abstract data sink objects.
*/
class BOTAN_DLL DataSink : public Filter
   {
   public:
      bool attachable() { return false; }
      DataSink() {}
      virtual ~DataSink() {}

      DataSink& operator=(const DataSink&) = delete;
      DataSink(const DataSink&) = delete;
   };

/**
* This class represents a data sink which writes its output to a stream.
*/
class BOTAN_DLL DataSink_Stream : public DataSink
   {
   public:
      std::string name() const { return identifier; }

      void write(const byte[], size_t);

      /**
      * Construct a DataSink_Stream from a stream.
      * @param stream the stream to write to
      * @param name identifier
      */
      DataSink_Stream(std::ostream& stream,
                      const std::string& name = "<std::ostream>");

      /**
      * Construct a DataSink_Stream from a stream.
      * @param pathname the name of the file to open a stream to
      * @param use_binary indicates whether to treat the file
      * as a binary file or not
      */
      DataSink_Stream(const std::string& pathname,
                      bool use_binary = false);

      ~DataSink_Stream();
   private:
      const std::string identifier;

      std::ostream* sink_p;
      std::ostream& sink;
   };

}


namespace Botan {

class BOTAN_DLL SQL_Database
   {
   public:
      class BOTAN_DLL Statement
         {
         public:
            /* Bind statement parameters */
            virtual void bind(int column, const std::string& str) = 0;

            virtual void bind(int column, size_t i) = 0;

            virtual void bind(int column, std::chrono::system_clock::time_point time) = 0;

            virtual void bind(int column, const std::vector<byte>& blob) = 0;

            /* Get output */
            virtual std::pair<const byte*, size_t> get_blob(int column) = 0;

            virtual size_t get_size_t(int column) = 0;

            /* Run to completion */
            virtual void spin() = 0;

            /* Maybe update */
            virtual bool step() = 0;

            virtual ~Statement() {}
         };

      /*
      * Create a new statement for execution.
      * Use ?1, ?2, ?3, etc for parameters to set later with bind
      */
      virtual std::shared_ptr<Statement> new_statement(const std::string& base_sql) const = 0;

      virtual size_t row_count(const std::string& table_name) = 0;

      virtual void create_table(const std::string& table_schema) = 0;

      virtual ~SQL_Database() {}
};

}


namespace Botan {

class BigInt;
class ASN1_Object;

/**
* General DER Encoding Object
*/
class BOTAN_DLL DER_Encoder
   {
   public:
      secure_vector<byte> get_contents();

      std::vector<byte> get_contents_unlocked()
         { return unlock(get_contents()); }

      DER_Encoder& start_cons(ASN1_Tag type_tag,
                              ASN1_Tag class_tag = UNIVERSAL);
      DER_Encoder& end_cons();

      DER_Encoder& start_explicit(u16bit type_tag);
      DER_Encoder& end_explicit();

      DER_Encoder& raw_bytes(const byte val[], size_t len);
      DER_Encoder& raw_bytes(const secure_vector<byte>& val);
      DER_Encoder& raw_bytes(const std::vector<byte>& val);

      DER_Encoder& encode_null();
      DER_Encoder& encode(bool b);
      DER_Encoder& encode(size_t s);
      DER_Encoder& encode(const BigInt& n);
      DER_Encoder& encode(const secure_vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const std::vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const byte val[], size_t len, ASN1_Tag real_type);

      DER_Encoder& encode(bool b,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(size_t s,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const BigInt& n,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const std::vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const secure_vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const byte v[], size_t len,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      template<typename T>
      DER_Encoder& encode_optional(const T& value, const T& default_value)
         {
         if(value != default_value)
            encode(value);
         return (*this);
         }

      template<typename T>
      DER_Encoder& encode_list(const std::vector<T>& values)
         {
         for(size_t i = 0; i != values.size(); ++i)
            encode(values[i]);
         return (*this);
         }

      DER_Encoder& encode(const ASN1_Object& obj);
      DER_Encoder& encode_if(bool pred, DER_Encoder& enc);
      DER_Encoder& encode_if(bool pred, const ASN1_Object& obj);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const byte rep[], size_t length);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, &rep[0], rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const secure_vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, &rep[0], rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::string& str);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              byte val);

   private:
      class DER_Sequence
         {
         public:
            ASN1_Tag tag_of() const;
            secure_vector<byte> get_contents();
            void add_bytes(const byte[], size_t);
            DER_Sequence(ASN1_Tag, ASN1_Tag);
         private:
            ASN1_Tag type_tag, class_tag;
            secure_vector<byte> contents;
            std::vector< secure_vector<byte> > set_contents;
         };

      secure_vector<byte> contents;
      std::vector<DER_Sequence> subsequences;
   };

}


namespace Botan {

/**
* DES
*/
class BOTAN_DLL DES : public Block_Cipher_Fixed_Params<8, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "DES"; }
      BlockCipher* clone() const { return new DES; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> round_key;
   };

/**
* Triple DES
*/
class BOTAN_DLL TripleDES : public Block_Cipher_Fixed_Params<8, 16, 24, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "TripleDES"; }
      BlockCipher* clone() const { return new TripleDES; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> round_key;
   };

/*
* DES Tables
*/
extern const u32bit DES_SPBOX1[256];
extern const u32bit DES_SPBOX2[256];
extern const u32bit DES_SPBOX3[256];
extern const u32bit DES_SPBOX4[256];
extern const u32bit DES_SPBOX5[256];
extern const u32bit DES_SPBOX6[256];
extern const u32bit DES_SPBOX7[256];
extern const u32bit DES_SPBOX8[256];

extern const u64bit DES_IPTAB1[256];
extern const u64bit DES_IPTAB2[256];
extern const u64bit DES_FPTAB1[256];
extern const u64bit DES_FPTAB2[256];

}


namespace Botan {

/**
* DESX
*/
class BOTAN_DLL DESX : public Block_Cipher_Fixed_Params<8, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "DESX"; }
      BlockCipher* clone() const { return new DESX; }
   private:
      void key_schedule(const byte[], size_t);
      secure_vector<byte> K1, K2;
      DES des;
   };

}


namespace Botan {

/**
* This class represents discrete logarithm groups. It holds a prime p,
* a prime q = (p-1)/2 and g = x^((p-1)/q) mod p.
*/
class BOTAN_DLL DL_Group
   {
   public:

      /**
      * Get the prime p.
      * @return prime p
      */
      const BigInt& get_p() const;

      /**
      * Get the prime q.
      * @return prime q
      */
      const BigInt& get_q() const;

      /**
      * Get the base g.
      * @return base g
      */
      const BigInt& get_g() const;

      /**
      * The DL group encoding format variants.
      */
      enum Format {
         ANSI_X9_42,
         ANSI_X9_57,
         PKCS_3,

         DSA_PARAMETERS = ANSI_X9_57,
         DH_PARAMETERS = ANSI_X9_42,
         X942_DH_PARAMETERS = ANSI_X9_42,
         PKCS3_DH_PARAMETERS = PKCS_3
      };

      /**
      * Determine the prime creation for DL groups.
      */
      enum PrimeType { Strong, Prime_Subgroup, DSA_Kosherizer };

      /**
      * Perform validity checks on the group.
      * @param rng the rng to use
      * @param strong whether to perform stronger by lengthier tests
      * @return true if the object is consistent, false otherwise
      */
      bool verify_group(RandomNumberGenerator& rng, bool strong) const;

      /**
      * Encode this group into a string using PEM encoding.
      * @param format the encoding format
      * @return string holding the PEM encoded group
      */
      std::string PEM_encode(Format format) const;

      /**
      * Encode this group into a string using DER encoding.
      * @param format the encoding format
      * @return string holding the DER encoded group
      */
      std::vector<byte> DER_encode(Format format) const;

      /**
      * Decode a DER/BER encoded group into this instance.
      * @param ber a vector containing the DER/BER encoded group
      * @param format the format of the encoded group
      */
      void BER_decode(const std::vector<byte>& ber,
                      Format format);

      /**
      * Decode a PEM encoded group into this instance.
      * @param pem the PEM encoding of the group
      */
      void PEM_decode(const std::string& pem);

      /**
      * Construct a DL group with uninitialized internal value.
      * Use this constructor is you wish to set the groups values
      * from a DER or PEM encoded group.
      */
      DL_Group();

      /**
      * Construct a DL group that is registered in the configuration.
      * @param name the name that is configured in the global configuration
      * for the desired group. If no configuration file is specified,
      * the default values from the file policy.cpp will be used. For instance,
      * use "modp/ietf/768" as name.
      */
      DL_Group(const std::string& name);

      /**
      * Create a new group randomly.
      * @param rng the random number generator to use
      * @param type specifies how the creation of primes p and q shall
      * be performed. If type=Strong, then p will be determined as a
      * safe prime, and q will be chosen as (p-1)/2. If
      * type=Prime_Subgroup and qbits = 0, then the size of q will be
      * determined according to the estimated difficulty of the DL
      * problem. If type=DSA_Kosherizer, DSA primes will be created.
      * @param pbits the number of bits of p
      * @param qbits the number of bits of q. Leave it as 0 to have
      * the value determined according to pbits.
      */
      DL_Group(RandomNumberGenerator& rng, PrimeType type,
               size_t pbits, size_t qbits = 0);

      /**
      * Create a DSA group with a given seed.
      * @param rng the random number generator to use
      * @param seed the seed to use to create the random primes
      * @param pbits the desired bit size of the prime p
      * @param qbits the desired bit size of the prime q.
      */
      DL_Group(RandomNumberGenerator& rng,
               const std::vector<byte>& seed,
               size_t pbits = 1024, size_t qbits = 0);

      /**
      * Create a DL group. The prime q will be determined according to p.
      * @param p the prime p
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& g);

      /**
      * Create a DL group.
      * @param p the prime p
      * @param q the prime q
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& q, const BigInt& g);

      /**
      * Return PEM representation of named DL group
      */
      static const char* PEM_for_named_group(const std::string& name);
   private:
      static BigInt make_dsa_generator(const BigInt&, const BigInt&);

      void init_check() const;
      void initialize(const BigInt&, const BigInt&, const BigInt&);
      bool initialized;
      BigInt p, q, g;
   };

}


namespace Botan {

/**
* PKCS #8 General Exception
*/
struct BOTAN_DLL PKCS8_Exception : public Decoding_Error
   {
   PKCS8_Exception(const std::string& error) :
      Decoding_Error("PKCS #8: " + error) {}
   };

/**
* This namespace contains functions for handling PKCS #8 private keys
*/
namespace PKCS8 {

/**
* BER encode a private key
* @param key the private key to encode
* @return BER encoded key
*/
BOTAN_DLL secure_vector<byte> BER_encode(const Private_Key& key);

/**
* Get a string containing a PEM encoded private key.
* @param key the key to encode
* @return encoded key
*/
BOTAN_DLL std::string PEM_encode(const Private_Key& key);

/**
* Encrypt a key using PKCS #8 encryption
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in binary BER form
*/
BOTAN_DLL std::vector<byte>
BER_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Get a string containing a PEM encoded private key, encrypting it with a
* password.
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in PEM form
*/
BOTAN_DLL std::string
PEM_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Load a key from a data source.
* @param source the data source providing the encoded key
* @param rng the rng to use
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(
  DataSource& source,
  RandomNumberGenerator& rng,
  std::function<std::string ()> get_passphrase);

/** Load a key from a data source.
* @param source the data source providing the encoded key
* @param rng the rng to use
* @param pass the passphrase to decrypt the key. Provide an empty
* string if the key is not encrypted
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng,
                                const std::string& pass = "");

/**
* Load a key from a file.
* @param filename the path to the file containing the encoded key
* @param rng the rng to use
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(
  const std::string& filename,
  RandomNumberGenerator& rng,
  std::function<std::string ()> get_passphrase);

/** Load a key from a file.
* @param filename the path to the file containing the encoded key
* @param rng the rng to use
* @param pass the passphrase to decrypt the key. Provide an empty
* string if the key is not encrypted
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng,
                                const std::string& pass = "");

/**
* Copy an existing encoded key object.
* @param key the key to copy
* @param rng the rng to use
* @return new copy of the key
*/
BOTAN_DLL Private_Key* copy_key(const Private_Key& key,
                                RandomNumberGenerator& rng);

}

}


namespace Botan {

/**
* This class represents discrete logarithm (DL) public keys.
*/
class BOTAN_DLL DL_Scheme_PublicKey : public virtual Public_Key
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      AlgorithmIdentifier algorithm_identifier() const;

      std::vector<byte> x509_subject_public_key() const;

      /**
      * Get the DL domain parameters of this key.
      * @return DL domain parameters of this key
      */
      const DL_Group& get_domain() const { return group; }

      /**
      * Get the public value y with y = g^x mod p where x is the secret key.
      */
      const BigInt& get_y() const { return y; }

      /**
      * Get the prime p of the underlying DL group.
      * @return prime p
      */
      const BigInt& group_p() const { return group.get_p(); }

      /**
      * Get the prime q of the underlying DL group.
      * @return prime q
      */
      const BigInt& group_q() const { return group.get_q(); }

      /**
      * Get the generator g of the underlying DL group.
      * @return generator g
      */
      const BigInt& group_g() const { return group.get_g(); }

      /**
      * Get the underlying groups encoding format.
      * @return encoding format
      */
      virtual DL_Group::Format group_format() const = 0;

      size_t estimated_strength() const override;

      DL_Scheme_PublicKey(const AlgorithmIdentifier& alg_id,
                          const secure_vector<byte>& key_bits,
                          DL_Group::Format group_format);

   protected:
      DL_Scheme_PublicKey() {}

      /**
      * The DL public key
      */
      BigInt y;

      /**
      * The DL group
      */
      DL_Group group;
   };

/**
* This class represents discrete logarithm (DL) private keys.
*/
class BOTAN_DLL DL_Scheme_PrivateKey : public virtual DL_Scheme_PublicKey,
                                       public virtual Private_Key
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      /**
      * Get the secret key x.
      * @return secret key
      */
      const BigInt& get_x() const { return x; }

      secure_vector<byte> pkcs8_private_key() const;

      DL_Scheme_PrivateKey(const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits,
                           DL_Group::Format group_format);

   protected:
      DL_Scheme_PrivateKey() {}

      /**
      * The DL private key
      */
      BigInt x;
   };

}


namespace Botan {

/**
* This class represents Diffie-Hellman public keys.
*/
class BOTAN_DLL DH_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "DH"; }

      std::vector<byte> public_value() const;
      size_t max_input_bits() const { return group_p().bits(); }

      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_42; }

      DH_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_42) {}

      /**
      * Construct a public key with the specified parameters.
      * @param grp the DL group to use in the key
      * @param y the public value y
      */
      DH_PublicKey(const DL_Group& grp, const BigInt& y);
   protected:
      DH_PublicKey() {}
   };

/**
* This class represents Diffie-Hellman private keys.
*/
class BOTAN_DLL DH_PrivateKey : public DH_PublicKey,
                                public PK_Key_Agreement_Key,
                                public virtual DL_Scheme_PrivateKey
   {
   public:
      std::vector<byte> public_value() const;

      /**
      * Load a DH private key
      * @param alg_id the algorithm id
      * @param key_bits the subject public key
      * @param rng a random number generator
      */
      DH_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits,
                    RandomNumberGenerator& rng);

      /**
      * Construct a private key with predetermined value.
      * @param rng random number generator to use
      * @param grp the group to be used in the key
      * @param x the key's secret value (or if zero, generate a new key)
      */
      DH_PrivateKey(RandomNumberGenerator& rng, const DL_Group& grp,
                    const BigInt& x = 0);
   };

}


namespace Botan {

/**
* BigInt Division
* @param x an integer
* @param y a non-zero integer
* @param q will be set to x / y
* @param r will be set to x % y
*/
void BOTAN_DLL divide(const BigInt& x,
                      const BigInt& y,
                      BigInt& q,
                      BigInt& r);

}


namespace Botan {

namespace PK_Ops {

template<typename Key>
struct PK_Spec
   {
   public:
      PK_Spec(const Key& key, const std::string& pad) :
         m_key(key), m_pad(pad) {}

      std::string algo_name() const { return m_key.algo_name(); }

      std::string as_string() const { return algo_name() + "/" + padding(); }

      const Key& key() const { return m_key; }
      const std::string& padding() const { return m_pad; }
   private:
      const Key& m_key;
      const std::string m_pad;
   };

/**
* Public key encryption interface
*/
class BOTAN_DLL Encryption
   {
   public:
      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> encrypt(const byte msg[], size_t msg_len, RandomNumberGenerator& rng) = 0;

      typedef PK_Spec<Public_Key> Spec;

      virtual ~Encryption() {}
   };

/**
* Public key decryption interface
*/
class BOTAN_DLL Decryption
   {
   public:
      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> decrypt(const byte msg[],  size_t msg_len) = 0;

      typedef PK_Spec<Private_Key> Spec;

      virtual ~Decryption() {}
   };

/**
* Public key signature creation interface
*/
class BOTAN_DLL Signature
   {
   public:
      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      /*
      * Perform a signature operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param rng a random number generator
      */
      virtual secure_vector<byte> sign(const byte msg[], size_t msg_len,
                                      RandomNumberGenerator& rng) = 0;

      typedef PK_Spec<Private_Key> Spec;

      virtual ~Signature() {}
   };

/**
* Public key signature verification interface
*/
class BOTAN_DLL Verification
   {
   public:
      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      /**
      * @return boolean specifying if this key type supports message
      * recovery and thus if you need to call verify() or verify_mr()
      */
      virtual bool with_recovery() const = 0;

      /*
      * Perform a signature check operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param sig the signature
      * @param sig_len the length of sig in bytes
      * @returns if signature is a valid one for message
      */
      virtual bool verify(const byte[], size_t,
                          const byte[], size_t)
         {
         throw Invalid_State("Message recovery required");
         }

      /*
      * Perform a signature operation (with message recovery)
      * Only call this if with_recovery() returns true
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @returns recovered message
      */
      virtual secure_vector<byte> verify_mr(const byte[],
                                           size_t)
         {
         throw Invalid_State("Message recovery not supported");
         }

      typedef PK_Spec<Public_Key> Spec;

      virtual ~Verification() {}
   };

/**
* A generic key agreement Operation (eg DH or ECDH)
*/
class BOTAN_DLL Key_Agreement
   {
   public:
      /*
      * Perform a key agreement operation
      * @param w the other key value
      * @param w_len the length of w in bytes
      * @returns the agreed key
      */
      virtual secure_vector<byte> agree(const byte w[], size_t w_len) = 0;

      typedef PK_Spec<Private_Key> Spec;

      virtual ~Key_Agreement() {}
   };

}

}


namespace Botan {

/**
* Encoding Method for Encryption
*/
class BOTAN_DLL EME
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Return the maximum input size in bytes we can support
      * @param keybits the size of the key in bits
      * @return upper bound of input in bytes
      */
      virtual size_t maximum_input_size(size_t keybits) const = 0;

      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<byte> encode(const byte in[],
                                size_t in_length,
                                size_t key_length,
                                RandomNumberGenerator& rng) const;

      /**
      * Encode an input
      * @param in the plaintext
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<byte> encode(const secure_vector<byte>& in,
                                size_t key_length,
                                RandomNumberGenerator& rng) const;

      /**
      * Decode an input
      * @param in the encoded plaintext
      * @param in_length length of encoded plaintext in bytes
      * @param key_length length of the key in bits
      * @return plaintext
      */
      secure_vector<byte> decode(const byte in[],
                                size_t in_length,
                                size_t key_length) const;

      /**
      * Decode an input
      * @param in the encoded plaintext
      * @param key_length length of the key in bits
      * @return plaintext
      */
      secure_vector<byte> decode(const secure_vector<byte>& in,
                                size_t key_length) const;

      virtual ~EME() {}
   private:
      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      virtual secure_vector<byte> pad(const byte in[],
                                     size_t in_length,
                                     size_t key_length,
                                     RandomNumberGenerator& rng) const = 0;

      /**
      * Decode an input
      * @param in the encoded plaintext
      * @param in_length length of encoded plaintext in bytes
      * @param key_length length of the key in bits
      * @return plaintext
      */
      virtual secure_vector<byte> unpad(const byte in[],
                                       size_t in_length,
                                       size_t key_length) const = 0;
   };

/**
* Factory method for EME (message-encoding methods for encryption) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EME*  get_eme(const std::string& algo_spec);

}


namespace Botan {

/**
* Encoding Method for Signatures, Appendix
*/
class BOTAN_DLL EMSA
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Add more data to the signature computation
      * @param input some data
      * @param length length of input in bytes
      */
      virtual void update(const byte input[], size_t length) = 0;

      /**
      * @return raw hash
      */
      virtual secure_vector<byte> raw_data() = 0;

      /**
      * Return the encoding of a message
      * @param msg the result of raw_data()
      * @param output_bits the desired output bit size
      * @param rng a random number generator
      * @return encoded signature
      */
      virtual secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                             size_t output_bits,
                                             RandomNumberGenerator& rng) = 0;

      /**
      * Verify the encoding
      * @param coded the received (coded) message representative
      * @param raw the computed (local, uncoded) message representative
      * @param key_bits the size of the key in bits
      * @return true if coded is a valid encoding of raw, otherwise false
      */
      virtual bool verify(const secure_vector<byte>& coded,
                          const secure_vector<byte>& raw,
                          size_t key_bits) = 0;
      virtual ~EMSA() {}
   };

/**
* Factory method for EMSA (message-encoding methods for signatures
* with appendix) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EMSA* get_emsa(const std::string& algo_spec);

}


namespace Botan {

/**
* Key Derivation Function
*/
class BOTAN_DLL KDF
   {
   public:
      virtual ~KDF() {}

      virtual std::string name() const = 0;

      virtual size_t kdf(byte key[], size_t key_len,
                         const byte secret[], size_t secret_len,
                         const byte salt[], size_t salt_len) const = 0;


      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         secure_vector<byte> key(key_len);
         key.resize(kdf(&key[0], key.size(), secret, secret_len, salt, salt_len));
         return key;
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, &secret[0], secret.size(),
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      template<typename Alloc, typename Alloc2>
      secure_vector<byte> derive_key(size_t key_len,
                                     const std::vector<byte, Alloc>& secret,
                                     const std::vector<byte, Alloc2>& salt) const
         {
         return derive_key(key_len,
                           &secret[0], secret.size(),
                           &salt[0], salt.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         return derive_key(key_len,
                           &secret[0], secret.size(),
                           salt, salt_len);
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, secret, secret_len,
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      virtual KDF* clone() const = 0;

      typedef SCAN_Name Spec;

   };

/**
* Factory method for KDF (key derivation function)
* @param algo_spec the name of the KDF to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL KDF*  get_kdf(const std::string& algo_spec);

}


namespace Botan {

/**
* The two types of signature format supported by Botan.
*/
enum Signature_Format { IEEE_1363, DER_SEQUENCE };

/**
* Enum marking if protection against fault attacks should be used
*/
enum Fault_Protection {
   ENABLE_FAULT_PROTECTION,
   DISABLE_FAULT_PROTECTION
};

/**
* Public Key Encryptor
*/
class BOTAN_DLL PK_Encryptor
   {
   public:

      /**
      * Encrypt a message.
      * @param in the message as a byte array
      * @param length the length of the above byte array
      * @param rng the random number source to use
      * @return encrypted message
      */
      std::vector<byte> encrypt(const byte in[], size_t length,
                                 RandomNumberGenerator& rng) const
         {
         return enc(in, length, rng);
         }

      /**
      * Encrypt a message.
      * @param in the message
      * @param rng the random number source to use
      * @return encrypted message
      */
      template<typename Alloc>
      std::vector<byte> encrypt(const std::vector<byte, Alloc>& in,
                                RandomNumberGenerator& rng) const
         {
         return enc(&in[0], in.size(), rng);
         }

      /**
      * Return the maximum allowed message size in bytes.
      * @return maximum message size in bytes
      */
      virtual size_t maximum_input_size() const = 0;

      PK_Encryptor() {}
      virtual ~PK_Encryptor() {}

      PK_Encryptor(const PK_Encryptor&) = delete;

      PK_Encryptor& operator=(const PK_Encryptor&) = delete;

   private:
      virtual std::vector<byte> enc(const byte[], size_t,
                                     RandomNumberGenerator&) const = 0;
   };

/**
* Public Key Decryptor
*/
class BOTAN_DLL PK_Decryptor
   {
   public:
      /**
      * Decrypt a ciphertext.
      * @param in the ciphertext as a byte array
      * @param length the length of the above byte array
      * @return decrypted message
      */
      secure_vector<byte> decrypt(const byte in[], size_t length) const
         {
         return dec(in, length);
         }

      /**
      * Decrypt a ciphertext.
      * @param in the ciphertext
      * @return decrypted message
      */
      template<typename Alloc>
      secure_vector<byte> decrypt(const std::vector<byte, Alloc>& in) const
         {
         return dec(&in[0], in.size());
         }

      PK_Decryptor() {}
      virtual ~PK_Decryptor() {}

      PK_Decryptor(const PK_Decryptor&) = delete;
      PK_Decryptor& operator=(const PK_Decryptor&) = delete;

   private:
      virtual secure_vector<byte> dec(const byte[], size_t) const = 0;
   };

/**
* Public Key Signer. Use the sign_message() functions for small
* messages. Use multiple calls update() to process large messages and
* generate the signature by finally calling signature().
*/
class BOTAN_DLL PK_Signer
   {
   public:
      /**
      * Sign a message.
      * @param in the message to sign as a byte array
      * @param length the length of the above byte array
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const byte in[], size_t length,
                                      RandomNumberGenerator& rng);

      /**
      * Sign a message.
      * @param in the message to sign
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const std::vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(&in[0], in.size(), rng); }

      std::vector<byte> sign_message(const secure_vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(&in[0], in.size(), rng); }

      /**
      * Add a message part (single byte).
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part.
      * @param in the message part to add as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte in[], size_t length);

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::vector<byte>& in) { update(&in[0], in.size()); }

      /**
      * Get the signature of the so far processed message (provided by the
      * calls to update()).
      * @param rng the rng to use
      * @return signature of the total message
      */
      std::vector<byte> signature(RandomNumberGenerator& rng);

      /**
      * Set the output format of the signature.
      * @param format the signature format to use
      */
      void set_output_format(Signature_Format format) { m_sig_format = format; }

      /**
      * Construct a PK Signer.
      * @param key the key to use inside this signer
      * @param emsa the EMSA to use
      * An example would be "EMSA1(SHA-224)".
      * @param format the signature format to use
      * @param prot says if fault protection should be enabled
      */
      PK_Signer(const Private_Key& key,
                const std::string& emsa,
                Signature_Format format = IEEE_1363,
                Fault_Protection prot = ENABLE_FAULT_PROTECTION);
   private:
      bool self_test_signature(const std::vector<byte>& msg,
                               const std::vector<byte>& sig) const;

      std::unique_ptr<PK_Ops::Signature> m_op;
      std::unique_ptr<PK_Ops::Verification> m_verify_op;
      std::unique_ptr<EMSA> m_emsa;
      Signature_Format m_sig_format;
   };

/**
* Public Key Verifier. Use the verify_message() functions for small
* messages. Use multiple calls update() to process large messages and
* verify the signature by finally calling check_signature().
*/
class BOTAN_DLL PK_Verifier
   {
   public:
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to, as a byte array
      * @param msg_length the length of the above byte array msg
      * @param sig the signature as a byte array
      * @param sig_length the length of the above byte array sig
      * @return true if the signature is valid
      */
      bool verify_message(const byte msg[], size_t msg_length,
                          const byte sig[], size_t sig_length);
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to
      * @param sig the signature
      * @return true if the signature is valid
      */
      template<typename Alloc, typename Alloc2>
      bool verify_message(const std::vector<byte, Alloc>& msg,
                          const std::vector<byte, Alloc2>& sig)
         {
         return verify_message(&msg[0], msg.size(),
                               &sig[0], sig.size());
         }

      /**
      * Add a message part (single byte) of the message corresponding to the
      * signature to be verified.
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param msg_part the new message part as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte msg_part[], size_t length);

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param in the new message part
      */
      void update(const std::vector<byte>& in)
         { update(&in[0], in.size()); }

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified as a byte array
      * @param length the length of the above byte array
      * @return true if the signature is valid, false otherwise
      */
      bool check_signature(const byte sig[], size_t length);

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified
      * @return true if the signature is valid, false otherwise
      */
      template<typename Alloc>
      bool check_signature(const std::vector<byte, Alloc>& sig)
         {
         return check_signature(&sig[0], sig.size());
         }

      /**
      * Set the format of the signatures fed to this verifier.
      * @param format the signature format to use
      */
      void set_input_format(Signature_Format format);

      /**
      * Construct a PK Verifier.
      * @param pub_key the public key to verify against
      * @param emsa the EMSA to use (eg "EMSA3(SHA-1)")
      * @param format the signature format to use
      */
      PK_Verifier(const Public_Key& pub_key,
                  const std::string& emsa,
                  Signature_Format format = IEEE_1363);
   private:
      bool validate_signature(const secure_vector<byte>& msg,
                              const byte sig[], size_t sig_len);

      std::unique_ptr<PK_Ops::Verification> m_op;
      std::unique_ptr<EMSA> m_emsa;
      Signature_Format m_sig_format;
   };

/**
* Key used for key agreement
*/
class BOTAN_DLL PK_Key_Agreement
   {
   public:

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[],
                              size_t in_len,
                              const byte params[],
                              size_t params_len) const;

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const byte params[],
                              size_t params_len) const
         {
         return derive_key(key_len, &in[0], in.size(),
                           params, params_len);
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[], size_t in_len,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in, in_len,
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const std::string& params = "") const
         {
         return derive_key(key_len, &in[0], in.size(),
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

      /**
      * Construct a PK Key Agreement.
      * @param key the key to use
      * @param kdf name of the KDF to use (or 'Raw' for no KDF)
      */
      PK_Key_Agreement(const Private_Key& key, const std::string& kdf);
   private:
      std::unique_ptr<PK_Ops::Key_Agreement> m_op;
      std::unique_ptr<KDF> m_kdf;
   };

/**
* Encryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Encryptor_EME : public PK_Encryptor
   {
   public:
      size_t maximum_input_size() const;

      /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param eme the EME to use
      */
      PK_Encryptor_EME(const Public_Key& key,
                       const std::string& eme);
   private:
      std::vector<byte> enc(const byte[], size_t,
                             RandomNumberGenerator& rng) const;

      std::unique_ptr<PK_Ops::Encryption> m_op;
      std::unique_ptr<EME> m_eme;
   };

/**
* Decryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Decryptor_EME : public PK_Decryptor
   {
   public:
     /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param eme the EME to use
      */
      PK_Decryptor_EME(const Private_Key& key,
                       const std::string& eme);
   private:
      secure_vector<byte> dec(const byte[], size_t) const;

      std::unique_ptr<PK_Ops::Decryption> m_op;
      std::unique_ptr<EME> m_eme;
   };

}


namespace Botan {

/**
* DLIES Encryption
*/
class BOTAN_DLL DLIES_Encryptor : public PK_Encryptor
   {
   public:
      DLIES_Encryptor(const PK_Key_Agreement_Key&,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      void set_other_key(const std::vector<byte>&);
   private:
      std::vector<byte> enc(const byte[], size_t,
                            RandomNumberGenerator&) const;

      size_t maximum_input_size() const;

      std::vector<byte> other_key, my_key;

      PK_Key_Agreement ka;
      std::unique_ptr<KDF> kdf;
      std::unique_ptr<MessageAuthenticationCode> mac;
      size_t mac_keylen;
   };

/**
* DLIES Decryption
*/
class BOTAN_DLL DLIES_Decryptor : public PK_Decryptor
   {
   public:
      DLIES_Decryptor(const PK_Key_Agreement_Key&,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

   private:
      secure_vector<byte> dec(const byte[], size_t) const;

      std::vector<byte> my_key;

      PK_Key_Agreement ka;
      std::unique_ptr<KDF> kdf;
      std::unique_ptr<MessageAuthenticationCode> mac;
      size_t mac_keylen;
   };

}


namespace Botan {

/**
* DSA Public Key
*/
class BOTAN_DLL DSA_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "DSA"; }

      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_57; }
      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return group_q().bytes(); }
      size_t max_input_bits() const { return group_q().bits(); }

      DSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_57)
         {
         }

      DSA_PublicKey(const DL_Group& group, const BigInt& y);
   protected:
      DSA_PublicKey() {}
   };

/**
* DSA Private Key
*/
class BOTAN_DLL DSA_PrivateKey : public DSA_PublicKey,
                                 public virtual DL_Scheme_PrivateKey
   {
   public:
      DSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits,
                     RandomNumberGenerator& rng);

      DSA_PrivateKey(RandomNumberGenerator& rng,
                     const DL_Group& group,
                     const BigInt& private_key = 0);

      bool check_key(RandomNumberGenerator& rng, bool strong) const;
   };

}


namespace Botan {

/**
* EAX base class
*/
class BOTAN_DLL EAX_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // EAX supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;
   protected:
      /**
      * @param cipher the cipher to use
      * @param tag_size is how big the auth tag will be
      */
      EAX_Mode(BlockCipher* cipher, size_t tag_size);

      size_t block_size() const { return m_cipher->block_size(); }

      size_t m_tag_size;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;

      secure_vector<byte> m_ad_mac;

      secure_vector<byte> m_nonce_mac;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* EAX Encryption
*/
class BOTAN_DLL EAX_Encryption : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Encryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* EAX Decryption
*/
class BOTAN_DLL EAX_Decryption : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Decryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* Exception thrown if you try to convert a zero point to an affine
* coordinate
*/
struct BOTAN_DLL Illegal_Transformation : public Exception
   {
   Illegal_Transformation(const std::string& err =
                          "Requested transformation is not possible") :
      Exception(err) {}
   };

/**
* Exception thrown if some form of illegal point is decoded
*/
struct BOTAN_DLL Illegal_Point : public Exception
   {
   Illegal_Point(const std::string& err = "Malformed ECP point detected") :
      Exception(err) {}
   };

/**
* This class represents one point on a curve of GF(p)
*/
class BOTAN_DLL PointGFp
   {
   public:
      enum Compression_Type {
         UNCOMPRESSED = 0,
         COMPRESSED   = 1,
         HYBRID       = 2
      };

      /**
      * Construct an uninitialized PointGFp
      */
      PointGFp() {}

      /**
      * Construct the zero point
      * @param curve The base curve
      */
      PointGFp(const CurveGFp& curve);

      /**
      * Copy constructor
      */
      PointGFp(const PointGFp&) = default;

      /**
      * Move Constructor
      */
      PointGFp(PointGFp&& other)
         {
         this->swap(other);
         }

      /**
      * Standard Assignment
      */
      PointGFp& operator=(const PointGFp&) = default;

      /**
      * Move Assignment
      */
      PointGFp& operator=(PointGFp&& other)
         {
         if(this != &other)
            this->swap(other);
         return (*this);
         }

      /**
      * Construct a point from its affine coordinates
      * @param curve the base curve
      * @param x affine x coordinate
      * @param y affine y coordinate
      */
      PointGFp(const CurveGFp& curve, const BigInt& x, const BigInt& y);

      /**
      * += Operator
      * @param rhs the PointGFp to add to the local value
      * @result resulting PointGFp
      */
      PointGFp& operator+=(const PointGFp& rhs);

      /**
      * -= Operator
      * @param rhs the PointGFp to subtract from the local value
      * @result resulting PointGFp
      */
      PointGFp& operator-=(const PointGFp& rhs);

      /**
      * *= Operator
      * @param scalar the PointGFp to multiply with *this
      * @result resulting PointGFp
      */
      PointGFp& operator*=(const BigInt& scalar);

      /**
      * Multiplication Operator
      * @param scalar the scalar value
      * @param point the point value
      * @return scalar*point on the curve
      */
      friend BOTAN_DLL PointGFp operator*(const BigInt& scalar, const PointGFp& point);

      /**
      * Multiexponentiation
      * @param p1 a point
      * @param z1 a scalar
      * @param p2 a point
      * @param z2 a scalar
      * @result (p1 * z1 + p2 * z2)
      */
      friend BOTAN_DLL PointGFp multi_exponentiate(
        const PointGFp& p1, const BigInt& z1,
        const PointGFp& p2, const BigInt& z2);

      /**
      * Negate this point
      * @return *this
      */
      PointGFp& negate()
         {
         if(!is_zero())
            coord_y = curve.get_p() - coord_y;
         return *this;
         }

      /**
      * Return base curve of this point
      * @result the curve over GF(p) of this point
      */
      const CurveGFp& get_curve() const { return curve; }

      /**
      * get affine x coordinate
      * @result affine x coordinate
      */
      BigInt get_affine_x() const;

      /**
      * get affine y coordinate
      * @result affine y coordinate
      */
      BigInt get_affine_y() const;

      /**
      * Is this the point at infinity?
      * @result true, if this point is at infinity, false otherwise.
      */
      bool is_zero() const
         { return (coord_x.is_zero() && coord_z.is_zero()); }

      /**
      * Checks whether the point is to be found on the underlying
      * curve; used to prevent fault attacks.
      * @return if the point is on the curve
      */
      bool on_the_curve() const;

      /**
      * swaps the states of *this and other, does not throw!
      * @param other the object to swap values with
      */
      void swap(PointGFp& other);

      /**
      * Equality operator
      */
      bool operator==(const PointGFp& other) const;
   private:

      BigInt curve_mult(const BigInt& x, const BigInt& y) const
         {
         BigInt z;
         curve.mul(z, x, y, ws);
         return z;
         }

      void curve_mult(BigInt& z, const BigInt& x, const BigInt& y) const
         {
         curve.mul(z, x, y, ws);
         }

      BigInt curve_sqr(const BigInt& x) const
         {
         BigInt z;
         curve.sqr(z, x, ws);
         return z;
         }

      void curve_sqr(BigInt& z, const BigInt& x) const
         {
         curve.sqr(z, x, ws);
         }

      /**
      * Point addition
      * @param workspace temp space, at least 11 elements
      */
      void add(const PointGFp& other, std::vector<BigInt>& workspace);

      /**
      * Point doubling
      * @param workspace temp space, at least 9 elements
      */
      void mult2(std::vector<BigInt>& workspace);

      CurveGFp curve;
      BigInt coord_x, coord_y, coord_z;
      mutable secure_vector<word> ws; // workspace for Montgomery
   };

// relational operators
inline bool operator!=(const PointGFp& lhs, const PointGFp& rhs)
   {
   return !(rhs == lhs);
   }

// arithmetic operators
inline PointGFp operator-(const PointGFp& lhs)
   {
   return PointGFp(lhs).negate();
   }

inline PointGFp operator+(const PointGFp& lhs, const PointGFp& rhs)
   {
   PointGFp tmp(lhs);
   return tmp += rhs;
   }

inline PointGFp operator-(const PointGFp& lhs, const PointGFp& rhs)
   {
   PointGFp tmp(lhs);
   return tmp -= rhs;
   }

inline PointGFp operator*(const PointGFp& point, const BigInt& scalar)
   {
   return scalar * point;
   }

// encoding and decoding
secure_vector<byte> BOTAN_DLL EC2OSP(const PointGFp& point, byte format);

PointGFp BOTAN_DLL OS2ECP(const byte data[], size_t data_len,
                          const CurveGFp& curve);

template<typename Alloc>
PointGFp OS2ECP(const std::vector<byte, Alloc>& data, const CurveGFp& curve)
   { return OS2ECP(&data[0], data.size(), curve); }

}

namespace std {

template<>
inline void swap<Botan::PointGFp>(Botan::PointGFp& x, Botan::PointGFp& y)
   { x.swap(y); }

}


namespace Botan {

/**
* This class represents elliptic curce domain parameters
*/
enum EC_Group_Encoding {
   EC_DOMPAR_ENC_EXPLICIT = 0,
   EC_DOMPAR_ENC_IMPLICITCA = 1,
   EC_DOMPAR_ENC_OID = 2
};

/**
* Class representing an elliptic curve
*/
class BOTAN_DLL EC_Group
   {
   public:

      /**
      * Construct Domain paramers from specified parameters
      * @param curve elliptic curve
      * @param base_point a base point
      * @param order the order of the base point
      * @param cofactor the cofactor
      */
      EC_Group(const CurveGFp& curve,
               const PointGFp& base_point,
               const BigInt& order,
               const BigInt& cofactor) :
         curve(curve),
         base_point(base_point),
         order(order),
         cofactor(cofactor),
         oid("")
         {}

      /**
      * Decode a BER encoded ECC domain parameter set
      * @param ber_encoding the bytes of the BER encoding
      */
      EC_Group(const std::vector<byte>& ber_encoding);

      /**
      * Create an EC domain by OID (or throw if unknown)
      * @param oid the OID of the EC domain to create
      */
      EC_Group(const OID& oid);

      /**
      * Create an EC domain from PEM encoding (as from PEM_encode), or
      * from an OID name (eg "secp256r1", or "1.2.840.10045.3.1.7")
      * @param pem_or_oid PEM-encoded data, or an OID
      */
      EC_Group(const std::string& pem_or_oid = "");

      /**
      * Create the DER encoding of this domain
      * @param form of encoding to use
      * @returns bytes encododed as DER
      */
      std::vector<byte> DER_encode(EC_Group_Encoding form) const;

      /**
      * Return the PEM encoding (always in explicit form)
      * @return string containing PEM data
      */
      std::string PEM_encode() const;

      /**
      * Return domain parameter curve
      * @result domain parameter curve
      */
      const CurveGFp& get_curve() const { return curve; }

      /**
      * Return group base point
      * @result base point
      */
      const PointGFp& get_base_point() const { return base_point; }

      /**
      * Return the order of the base point
      * @result order of the base point
      */
      const BigInt& get_order() const { return order; }

      /**
      * Return the cofactor
      * @result the cofactor
      */
      const BigInt& get_cofactor() const { return cofactor; }

      bool initialized() const { return !base_point.is_zero(); }

      /**
      * Return the OID of these domain parameters
      * @result the OID
      */
      std::string get_oid() const { return oid; }

      bool operator==(const EC_Group& other) const
         {
         return ((get_curve() == other.get_curve()) &&
                 (get_base_point() == other.get_base_point()) &&
                 (get_order() == other.get_order()) &&
                 (get_cofactor() == other.get_cofactor()));
         }

      /**
      * Return PEM representation of named EC group
      */
      static const char* PEM_for_named_group(const std::string& name);

   private:
      CurveGFp curve;
      PointGFp base_point;
      BigInt order, cofactor;
      std::string oid;
   };

inline bool operator!=(const EC_Group& lhs,
                       const EC_Group& rhs)
   {
   return !(lhs == rhs);
   }

// For compatability with 1.8
typedef EC_Group EC_Domain_Params;

}


namespace Botan {

/**
* ECB mode
*/
class BOTAN_DLL ECB_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      ECB_Mode(BlockCipher* cipher, BlockCipherModePaddingMethod* padding);

      const BlockCipher& cipher() const { return *m_cipher; }

      const BlockCipherModePaddingMethod& padding() const { return *m_padding; }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;
      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<BlockCipherModePaddingMethod> m_padding;
   };

/**
* ECB Encryption
*/
class BOTAN_DLL ECB_Encryption : public ECB_Mode
   {
   public:
      ECB_Encryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         ECB_Mode(cipher, padding) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   };

/**
* ECB Decryption
*/
class BOTAN_DLL ECB_Decryption : public ECB_Mode
   {
   public:
      ECB_Decryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         ECB_Mode(cipher, padding) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   };

}


namespace Botan {

/**
* This class represents abstract ECC public keys. When encoding a key
* via an encoder that can be accessed via the corresponding member
* functions, the key will decide upon its internally stored encoding
* information whether to encode itself with or without domain
* parameters, or using the domain parameter oid. Furthermore, a public
* key without domain parameters can be decoded. In that case, it
* cannot be used for verification until its domain parameters are set
* by calling the corresponding member function.
*/
class BOTAN_DLL EC_PublicKey : public virtual Public_Key
   {
   public:
      EC_PublicKey(const EC_Group& dom_par,
                   const PointGFp& pub_point);

      EC_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits);

      /**
      * Get the public point of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the public point of this key
      */
      const PointGFp& public_point() const { return public_key; }

      AlgorithmIdentifier algorithm_identifier() const;

      std::vector<byte> x509_subject_public_key() const;

      bool check_key(RandomNumberGenerator& rng,
                     bool strong) const;

      /**
      * Get the domain parameters of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the domain parameters of this key
      */
      const EC_Group& domain() const { return domain_params; }

      /**
      * Set the domain parameter encoding to be used when encoding this key.
      * @param enc the encoding to use
      */
      void set_parameter_encoding(EC_Group_Encoding enc);

      /**
      * Return the DER encoding of this keys domain in whatever format
      * is preset for this particular key
      */
      std::vector<byte> DER_domain() const
         { return domain().DER_encode(domain_format()); }

      /**
      * Get the domain parameter encoding to be used when encoding this key.
      * @result the encoding to use
      */
      EC_Group_Encoding domain_format() const
         { return domain_encoding; }

      size_t estimated_strength() const override;

   protected:
      EC_PublicKey() : domain_encoding(EC_DOMPAR_ENC_EXPLICIT) {}

      EC_Group domain_params;
      PointGFp public_key;
      EC_Group_Encoding domain_encoding;
   };

/**
* This abstract class represents ECC private keys
*/
class BOTAN_DLL EC_PrivateKey : public virtual EC_PublicKey,
                                public virtual Private_Key
   {
   public:
     EC_PrivateKey(RandomNumberGenerator& rng,
                   const EC_Group& domain,
                   const BigInt& private_key);

      EC_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits);

      secure_vector<byte> pkcs8_private_key() const;

      /**
      * Get the private key value of this key object.
      * @result the private key value of this key object
      */
      const BigInt& private_value() const;
   protected:
      EC_PrivateKey() {}

      BigInt private_key;
   };

}


namespace Botan {

/**
* This class represents ECDH Public Keys.
*/
class BOTAN_DLL ECDH_PublicKey : public virtual EC_PublicKey
   {
   public:

      ECDH_PublicKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECDH_PublicKey(const EC_Group& dom_par,
                     const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Get this keys algorithm name.
      * @return this keys algorithm name
      */
      std::string algo_name() const { return "ECDH"; }

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.

      * @return maximum number of input bits
      */
      size_t max_input_bits() const { return domain().get_order().bits(); }

      /**
      * @return public point value
      */
      std::vector<byte> public_value() const
         { return unlock(EC2OSP(public_point(), PointGFp::UNCOMPRESSED)); }

   protected:
      ECDH_PublicKey() {}
   };

/**
* This class represents ECDH Private Keys.
*/
class BOTAN_DLL ECDH_PrivateKey : public ECDH_PublicKey,
                                  public EC_PrivateKey,
                                  public PK_Key_Agreement_Key
   {
   public:

      ECDH_PrivateKey(const AlgorithmIdentifier& alg_id,
                      const secure_vector<byte>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key; if zero, a new random key is generated
      */
      ECDH_PrivateKey(RandomNumberGenerator& rng,
                      const EC_Group& domain,
                      const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      std::vector<byte> public_value() const
         { return ECDH_PublicKey::public_value(); }
   };

}


namespace Botan {

/**
* This class represents ECDSA Public Keys.
*/
class BOTAN_DLL ECDSA_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECDSA_PublicKey(const EC_Group& dom_par,
                      const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      ECDSA_PublicKey(const AlgorithmIdentifier& alg_id,
                      const secure_vector<byte>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name ("ECDSA")
      */
      std::string algo_name() const { return "ECDSA"; }

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const { return domain().get_order().bits(); }

      size_t message_parts() const { return 2; }

      size_t message_part_size() const
         { return domain().get_order().bytes(); }

   protected:
      ECDSA_PublicKey() {}
   };

/**
* This class represents ECDSA Private Keys
*/
class BOTAN_DLL ECDSA_PrivateKey : public ECDSA_PublicKey,
                                   public EC_PrivateKey
   {
   public:

      /**
      * Load a private key
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ECDSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                       const secure_vector<byte>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a ney random key)
      */
      ECDSA_PrivateKey(RandomNumberGenerator& rng,
                       const EC_Group& domain,
                       const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      bool check_key(RandomNumberGenerator& rng, bool) const;
   };

}


namespace Botan {

/**
* ElGamal Public Key
*/
class BOTAN_DLL ElGamal_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "ElGamal"; }
      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_42; }

      size_t max_input_bits() const { return (group_p().bits() - 1); }

      ElGamal_PublicKey(const AlgorithmIdentifier& alg_id,
                        const secure_vector<byte>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_42)
         {}

      ElGamal_PublicKey(const DL_Group& group, const BigInt& y);
   protected:
      ElGamal_PublicKey() {}
   };

/**
* ElGamal Private Key
*/
class BOTAN_DLL ElGamal_PrivateKey : public ElGamal_PublicKey,
                                     public virtual DL_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      ElGamal_PrivateKey(const AlgorithmIdentifier& alg_id,
                         const secure_vector<byte>& key_bits,
                         RandomNumberGenerator& rng);

      ElGamal_PrivateKey(RandomNumberGenerator& rng,
                         const DL_Group& group,
                         const BigInt& priv_key = 0);
   };

}


namespace Botan {

/**
* EME from PKCS #1 v1.5
*/
class BOTAN_DLL EME_PKCS1v15 : public EME
   {
   public:
      size_t maximum_input_size(size_t) const;
   private:
      secure_vector<byte> pad(const byte[], size_t, size_t,
                             RandomNumberGenerator&) const;
      secure_vector<byte> unpad(const byte[], size_t, size_t) const;
   };

}


namespace Botan {

/**
* EMSA1 from IEEE 1363
* Essentially, sign the hash directly
*/
class BOTAN_DLL EMSA1 : public EMSA
   {
   public:
      /**
      * @param hash the hash function to use
      */
      EMSA1(HashFunction* hash) : m_hash(hash) {}

   protected:
      size_t hash_output_length() const { return m_hash->output_length(); }
   private:
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits);

      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* EMSA1_BSI is a variant of EMSA1 specified by the BSI. It accepts
* only hash values which are less or equal than the maximum key
* length. The implementation comes from InSiTo
*/
class BOTAN_DLL EMSA1_BSI : public EMSA1
   {
   public:
      /**
      * @param hash the hash object to use
      */
      EMSA1_BSI(HashFunction* hash) : EMSA1(hash) {}
   private:
      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);
   };

}


namespace Botan {

/**
* PKCS #1 v1.5 signature padding
* aka PKCS #1 block type 1
* aka EMSA3 from IEEE 1363
*/
class BOTAN_DLL EMSA_PKCS1v15 : public EMSA
   {
   public:
      /**
      * @param hash the hash object to use
      */
      EMSA_PKCS1v15(HashFunction* hash);

      void update(const byte[], size_t);

      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);
   private:
      std::unique_ptr<HashFunction> m_hash;
      std::vector<byte> m_hash_id;
   };

/**
* EMSA_PKCS1v15_Raw which is EMSA_PKCS1v15 without a hash or digest id
* (which according to QCA docs is "identical to PKCS#11's CKM_RSA_PKCS
* mechanism", something I have not confirmed)
*/
class BOTAN_DLL EMSA_PKCS1v15_Raw : public EMSA
   {
   public:
      void update(const byte[], size_t);

      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);

   private:
      secure_vector<byte> message;
   };

}


namespace Botan {

/**
* EMSA-Raw - sign inputs directly
* Don't use this unless you know what you are doing.
*/
class BOTAN_DLL EMSA_Raw : public EMSA
   {
   private:
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator&);
      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);

      secure_vector<byte> message;
   };

}


namespace Botan {

/**
* EMSA from X9.31 (EMSA2 in IEEE 1363)
* Useful for Rabin-Williams, also sometimes used with RSA in
* odd protocols.
*/
class BOTAN_DLL EMSA_X931 : public EMSA
   {
   public:
      /**
      * @param hash the hash object to use
      */
      EMSA_X931(HashFunction* hash);
   private:
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);

      secure_vector<byte> m_empty_hash;
      std::unique_ptr<HashFunction> m_hash;
      byte m_hash_id;
   };

}

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
* TODO:
* - Better error reporting
* - User callback for exception logging
* - Doxygen comments for all functions/params
* - X.509 certs and PKIX path validation goo
* - TLS
*/

/*
* Versioning
*/
BOTAN_DLL uint32_t botan_ffi_api_version();

BOTAN_DLL const char* botan_version_string();
BOTAN_DLL uint32_t botan_version_major();
BOTAN_DLL uint32_t botan_version_minor();
BOTAN_DLL uint32_t botan_version_patch();
BOTAN_DLL uint32_t botan_version_datestamp();

/*
* Error handling
*/
#define BOTAN_FFI_ERROR_EXCEPTION_THROWN (-20)
#define BOTAN_FFI_ERROR_BAD_FLAG (-30)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)

//const char* botan_error_description(int err);

/*
* Utility
*/
BOTAN_DLL int botan_same_mem(const uint8_t* x, const uint8_t* y, size_t len);

/*
* RNG
*/
typedef struct botan_rng_struct* botan_rng_t;

BOTAN_DLL int botan_rng_init(botan_rng_t* rng, const char* rng_type);
BOTAN_DLL int botan_rng_get(botan_rng_t rng, uint8_t* out, size_t out_len);
BOTAN_DLL int botan_rng_reseed(botan_rng_t rng, size_t bits);
BOTAN_DLL int botan_rng_destroy(botan_rng_t rng);

/*
* Hashing
*/
typedef struct botan_hash_struct* botan_hash_t;

BOTAN_DLL int botan_hash_init(botan_hash_t* hash, const char* hash_name, uint32_t flags);
BOTAN_DLL int botan_hash_output_length(botan_hash_t hash, size_t* output_length);
BOTAN_DLL int botan_hash_update(botan_hash_t hash, const uint8_t* in, size_t in_len);
BOTAN_DLL int botan_hash_final(botan_hash_t hash, uint8_t out[]);
BOTAN_DLL int botan_hash_clear(botan_hash_t hash);
BOTAN_DLL int botan_hash_destroy(botan_hash_t hash);

/*
* Message Authentication
*/
typedef struct botan_mac_struct* botan_mac_t;

BOTAN_DLL int botan_mac_init(botan_mac_t* mac, const char* mac_name, uint32_t flags);
BOTAN_DLL int botan_mac_output_length(botan_mac_t mac, size_t* output_length);
BOTAN_DLL int botan_mac_set_key(botan_mac_t mac, const uint8_t* key, size_t key_len);
BOTAN_DLL int botan_mac_update(botan_mac_t mac, const uint8_t* buf, size_t len);
BOTAN_DLL int botan_mac_final(botan_mac_t mac, uint8_t out[]);
BOTAN_DLL int botan_mac_clear(botan_mac_t hash);
BOTAN_DLL int botan_mac_destroy(botan_mac_t mac);

/*
* Cipher modes
*/
typedef struct botan_cipher_struct* botan_cipher_t;

#define BOTAN_CIPHER_INIT_FLAG_MASK_DIRECTION 1
#define BOTAN_CIPHER_INIT_FLAG_ENCRYPT 0
#define BOTAN_CIPHER_INIT_FLAG_DECRYPT 1

BOTAN_DLL int botan_cipher_init(botan_cipher_t* cipher, const char* name, uint32_t flags);

BOTAN_DLL int botan_cipher_valid_nonce_length(botan_cipher_t cipher, size_t nl);
BOTAN_DLL int botan_cipher_get_tag_length(botan_cipher_t cipher, size_t* tag_size);
BOTAN_DLL int botan_cipher_get_default_nonce_length(botan_cipher_t cipher, size_t* nl);

BOTAN_DLL int botan_cipher_set_key(botan_cipher_t cipher,
                                   const uint8_t* key, size_t key_len);

BOTAN_DLL int botan_cipher_set_associated_data(botan_cipher_t cipher,
                                               const uint8_t* ad, size_t ad_len);

BOTAN_DLL int botan_cipher_start(botan_cipher_t cipher,
                                 const uint8_t* nonce, size_t nonce_len);

#define BOTAN_CIPHER_UPDATE_FLAG_FINAL (1U << 0)

BOTAN_DLL int botan_cipher_update(botan_cipher_t cipher,
                                  uint32_t flags,
                                  uint8_t output[],
                                  size_t output_size,
                                  size_t* output_written,
                                  const uint8_t input_bytes[],
                                  size_t input_size,
                                  size_t* input_consumed);

BOTAN_DLL int botan_cipher_clear(botan_cipher_t hash);
BOTAN_DLL int botan_cipher_destroy(botan_cipher_t cipher);

/*
* PBKDF
*/
BOTAN_DLL int botan_pbkdf(const char* pbkdf_algo,
                          uint8_t out[], size_t out_len,
                          const char* password,
                          const uint8_t salt[], size_t salt_len,
                          size_t iterations);

BOTAN_DLL int botan_pbkdf_timed(const char* pbkdf_algo,
                                uint8_t out[], size_t out_len,
                                const char* password,
                                const uint8_t salt[], size_t salt_len,
                                size_t milliseconds_to_run,
                                size_t* out_iterations_used);

/*
* KDF
*/
BOTAN_DLL int botan_kdf(const char* kdf_algo,
                        uint8_t out[], size_t out_len,
                        const uint8_t secret[], size_t secret_len,
                        const uint8_t salt[], size_t salt_len);

/*
* Bcrypt
*/
#if defined(BOTAN_HAS_BCRYPT)

BOTAN_DLL int botan_bcrypt_generate(uint8_t* out, size_t* out_len,
                                    const char* pass,
                                    botan_rng_t rng,
                                    size_t work_factor,
                                    uint32_t flags);

/**
* Returns 0 if if this password/hash combination is valid
* Returns 1 if the combination is not valid (but otherwise well formed)
* Returns negative on error
*/
BOTAN_DLL int botan_bcrypt_is_valid(const char* pass, const char* hash);

#endif

/*
* Public/private key creation, import, ...
*/
typedef struct botan_privkey_struct* botan_privkey_t;

BOTAN_DLL int botan_privkey_create_rsa(botan_privkey_t* key, botan_rng_t rng, size_t n_bits);
//BOTAN_DLL int botan_privkey_create_dsa(botan_privkey_t* key, botan_rng_t rng, size_t p_bits, size_t q_bits);
//BOTAN_DLL int botan_privkey_create_dh(botan_privkey_t* key, botan_rng_t rng, size_t p_bits);
BOTAN_DLL int botan_privkey_create_ecdsa(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_ecdh(botan_privkey_t* key, botan_rng_t rng, const char* params);
//BOTAN_DLL int botan_privkey_create_mceliece(botan_privkey_t* key, botan_rng_t rng, size_t n, size_t t);

/*
* Input currently assumed to be PKCS #8 structure;
* Set password to NULL to indicate no encryption expected
*/
BOTAN_DLL int botan_privkey_load(botan_privkey_t* key, botan_rng_t rng,
                                 const uint8_t bits[], size_t len,
                                 const char* password);

BOTAN_DLL int botan_privkey_destroy(botan_privkey_t key);

#define BOTAN_PRIVKEY_EXPORT_FLAG_DER 0
#define BOTAN_PRIVKEY_EXPORT_FLAG_PEM 1

/*
* On input *out_len is number of bytes in out[]
* On output *out_len is number of bytes written (or required)
* If out is not big enough no output is written, *out_len is set and 1 is returned
* Returns 0 on success and sets
* If some other error occurs a negative integer is returned.
*/
BOTAN_DLL int botan_privkey_export(botan_privkey_t key,
                                   uint8_t out[], size_t* out_len,
                                   uint32_t flags);

/*
* Set encryption_algo to NULL or "" to have the library choose a default (recommended)
*/
BOTAN_DLL int botan_privkey_export_encrypted(botan_privkey_t key,
                                             uint8_t out[], size_t* out_len,
                                             botan_rng_t rng,
                                             const char* passphrase,
                                             const char* encryption_algo,
                                             uint32_t flags);

typedef struct botan_pubkey_struct* botan_pubkey_t;

BOTAN_DLL int botan_pubkey_load(botan_pubkey_t* key, const uint8_t bits[], size_t len);

BOTAN_DLL int botan_privkey_export_pubkey(botan_pubkey_t* out, botan_privkey_t in);

BOTAN_DLL int botan_pubkey_export(botan_pubkey_t key, uint8_t out[], size_t* out_len, uint32_t flags);

BOTAN_DLL int botan_pubkey_algo_name(botan_pubkey_t key, char out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_estimated_strength(botan_pubkey_t key, size_t* estimate);

BOTAN_DLL int botan_pubkey_fingerprint(botan_pubkey_t key, const char* hash,
                                       uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_destroy(botan_privkey_t key);


/*
* Public Key Encryption
*/
typedef struct botan_pk_op_encrypt_struct* botan_pk_op_encrypt_t;

BOTAN_DLL int botan_pk_op_encrypt_create(botan_pk_op_encrypt_t* op,
                                         botan_pubkey_t key,
                                         const char* padding,
                                         uint32_t flags);

BOTAN_DLL int botan_pk_op_encrypt_destroy(botan_pk_op_encrypt_t op);

BOTAN_DLL int botan_pk_op_encrypt(botan_pk_op_encrypt_t op,
                                  botan_rng_t rng,
                                  uint8_t out[], size_t* out_len,
                                  const uint8_t plaintext[], size_t plaintext_len);

/*
* Public Key Decryption
*/
typedef struct botan_pk_op_decrypt_struct* botan_pk_op_decrypt_t;

BOTAN_DLL int botan_pk_op_decrypt_create(botan_pk_op_decrypt_t* op,
                                         botan_privkey_t key,
                                         const char* padding,
                                         uint32_t flags);
BOTAN_DLL int botan_pk_op_decrypt_destroy(botan_pk_op_decrypt_t op);

BOTAN_DLL int botan_pk_op_decrypt(botan_pk_op_decrypt_t op,
                                  uint8_t out[], size_t* out_len,
                                  uint8_t ciphertext[], size_t ciphertext_len);

/*
* Signature Generation
*/
typedef struct botan_pk_op_sign_struct* botan_pk_op_sign_t;

BOTAN_DLL int botan_pk_op_sign_create(botan_pk_op_sign_t* op,
                                      botan_privkey_t key,
                                      const char* hash_and_padding,
                                      uint32_t flags);
BOTAN_DLL int botan_pk_op_sign_destroy(botan_pk_op_sign_t op);

BOTAN_DLL int botan_pk_op_sign_update(botan_pk_op_sign_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_sign_finish(botan_pk_op_sign_t op, botan_rng_t rng,
                                      uint8_t sig[], size_t* sig_len);

/*
* Signature Verification
*/
typedef struct botan_pk_op_verify_struct* botan_pk_op_verify_t;

BOTAN_DLL int botan_pk_op_verify_create(botan_pk_op_verify_t* op,
                                        botan_pubkey_t key,
                                        const char* hash_and_padding,
                                        uint32_t flags);
BOTAN_DLL int botan_pk_op_verify_destroy(botan_pk_op_verify_t op);

BOTAN_DLL int botan_pk_op_verify_update(botan_pk_op_verify_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_verify_finish(botan_pk_op_verify_t op, const uint8_t sig[], size_t sig_len);

/*
* Key Agreement
*/
typedef struct botan_pk_op_ka_struct* botan_pk_op_ka_t;

BOTAN_DLL int botan_pk_op_key_agreement_create(botan_pk_op_ka_t* op,
                                               botan_privkey_t key,
                                               const char* kdf,
                                               uint32_t flags);
BOTAN_DLL int botan_pk_op_key_agreement_destroy(botan_pk_op_ka_t op);

BOTAN_DLL int botan_pk_op_key_agreement_export_public(botan_privkey_t key,
                                                      uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pk_op_key_agreement(botan_pk_op_ka_t op,
                                        uint8_t out[], size_t* out_len,
                                        const uint8_t other_key[], size_t other_key_len,
                                        const uint8_t salt[], size_t salt_len);

/*
* TLS (WIP)
*/
#if defined(BOTAN_HAS_TLS) && 0

typedef struct botan_tls_session_struct* botan_tls_session_t;

BOTAN_DLL int botan_tls_session_get_version(botan_tls_session_t* session, uint16_t* tls_version);
BOTAN_DLL int botan_tls_session_get_ciphersuite(botan_tls_session_t* session, uint16_t* ciphersuite);
// TODO: peer certs, validation, ...

typedef struct botan_tls_channel_struct* botan_tls_channel_t;

typedef void (*botan_tls_channel_output_fn)(void*, const uint8_t*, size_t);
typedef void (*botan_tls_channel_data_cb)(void*, const uint8_t*, size_t);
typedef void (*botan_tls_channel_alert_cb)(void*, uint16_t, const char*);
typedef void (*botan_tls_channel_session_established)(void*, botan_tls_session_t);

BOTAN_DLL int botan_tls_channel_init_client(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb,
                                            const char* server_name);

BOTAN_DLL int botan_tls_channel_init_server(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb);

BOTAN_DLL int botan_tls_channel_received_data(botan_tls_channel_t chan,
                                              const uint8_t input[], size_t len);

BOTAN_DLL int botan_tls_channel_send(botan_tls_channel_t chan,
                                     const uint8_t input[], size_t len);

BOTAN_DLL int botan_tls_channel_close(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_destroy(botan_tls_channel_t chan);

#endif

#ifdef __cplusplus
}
#endif



namespace Botan {

/**
* This class represents keyed filters, i.e. filters that have to be
* fed with a key in order to function.
*/
class BOTAN_DLL Keyed_Filter : public Filter
   {
   public:
      /**
      * Set the key of this filter
      * @param key the key to use
      */
      virtual void set_key(const SymmetricKey& key) = 0;

      /**
      * Set the initialization vector of this filter. Note: you should
      * call set_iv() only after you have called set_key()
      * @param iv the initialization vector to use
      */
      virtual void set_iv(const InitializationVector& iv);

      /**
      * Check whether a key length is valid for this filter
      * @param length the key length to be checked for validity
      * @return true if the key length is valid, false otherwise
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether an IV length is valid for this filter
      * @param length the IV length to be checked for validity
      * @return true if the IV length is valid, false otherwise
      */
      virtual bool valid_iv_length(size_t length) const
         { return (length == 0); }
   };



/*
* Get a cipher object
*/

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param iv the initialization vector to be used
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to newly allocated encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   const InitializationVector& iv,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters. No key will be
* set in the filter.
*
* @param algo_spec the name of the desired cipher
* @param direction determines whether the filter will be an encrypting or
* decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   Cipher_Dir direction);

}



#if defined(BOTAN_HAS_CODEC_FILTERS)

namespace Botan {

/**
* Converts arbitrary binary data to hex strings, optionally with
* newlines inserted
*/
class BOTAN_DLL Hex_Encoder : public Filter
   {
   public:
      /**
      * Whether to use uppercase or lowercase letters for the encoded string.
      */
      enum Case { Uppercase, Lowercase };

      std::string name() const { return "Hex_Encoder"; }

      void write(const byte in[], size_t length);
      void end_msg();

      /**
      * Create a hex encoder.
      * @param the_case the case to use in the encoded strings.
      */
      Hex_Encoder(Case the_case);

      /**
      * Create a hex encoder.
      * @param newlines should newlines be used
      * @param line_length if newlines are used, how long are lines
      * @param the_case the case to use in the encoded strings
      */
      Hex_Encoder(bool newlines = false,
                  size_t line_length = 72,
                  Case the_case = Uppercase);
   private:
      void encode_and_send(const byte[], size_t);

      const Case casing;
      const size_t line_length;
      std::vector<byte> in, out;
      size_t position, counter;
   };

/**
* Converts hex strings to bytes
*/
class BOTAN_DLL Hex_Decoder : public Filter
   {
   public:
      std::string name() const { return "Hex_Decoder"; }

      void write(const byte[], size_t);
      void end_msg();

      /**
      * Construct a Hex Decoder using the specified
      * character checking.
      * @param checking the checking to use during decoding.
      */
      Hex_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking checking;
      std::vector<byte> in, out;
      size_t position;
   };

}

#endif

namespace Botan {

/**
* Stream Cipher Filter
*/
class BOTAN_DLL StreamCipher_Filter : public Keyed_Filter
   {
   public:

      std::string name() const { return m_cipher->name(); }

      /**
      * Write input data
      * @param input data
      * @param input_len length of input in bytes
      */
      void write(const byte input[], size_t input_len);

      bool valid_iv_length(size_t iv_len) const
         { return m_cipher->valid_iv_length(iv_len); }

      /**
      * Set the initialization vector for this filter.
      * @param iv the initialization vector to set
      */
      void set_iv(const InitializationVector& iv)
         {
         m_cipher->set_iv(iv.begin(), iv.length());
         }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) { m_cipher->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      */
      StreamCipher_Filter(StreamCipher* cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      */
      StreamCipher_Filter(const std::string& cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(const std::string& cipher, const SymmetricKey& key);
   private:
      secure_vector<byte> m_buffer;
      std::unique_ptr<StreamCipher> m_cipher;
   };

/**
* Hash Filter.
*/
class BOTAN_DLL Hash_Filter : public Filter
   {
   public:
      void write(const byte input[], size_t len) { m_hash->update(input, len); }
      void end_msg();

      std::string name() const { return m_hash->name(); }

      /**
      * Construct a hash filter.
      * @param hash_fun the hash function to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(HashFunction* hash, size_t len = 0) :
         m_hash(hash), m_out_len(len) {}

      /**
      * Construct a hash filter.
      * @param request the name of the hash algorithm to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(const std::string& request, size_t len = 0);

   private:
      std::unique_ptr<HashFunction> m_hash;
      const size_t m_out_len;
   };

/**
* MessageAuthenticationCode Filter.
*/
class BOTAN_DLL MAC_Filter : public Keyed_Filter
   {
   public:
      void write(const byte input[], size_t len) { m_mac->update(input, len); }
      void end_msg();

      std::string name() const { return m_mac->name(); }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) { m_mac->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_mac->key_spec(); }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the MAC to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         }

      /**
      * Construct a MAC filter.
      * @param mac the MAC to use
      * @param key the MAC key to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 const SymmetricKey& key,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         m_mac->set_key(key);
         }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the name of the MAC to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, size_t len = 0);

      /**
      * Construct a MAC filter.
      * @param mac the name of the MAC to use
      * @param key the MAC key to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, const SymmetricKey& key,
                 size_t len = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_out_len;
   };

}


namespace Botan {

namespace FPE {

/**
* Format Preserving Encryption using the scheme FE1 from the paper
* "Format-Preserving Encryption" by Bellare, Rogaway, et al
* (http://eprint.iacr.org/2009/251)
*
* Encrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the plaintext as a BigInt
* @param key a random key
* @param tweak will modify the ciphertext (think of as an IV)
*/
BigInt BOTAN_DLL fe1_encrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<byte>& tweak);

/**
* Decrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the ciphertext as a BigInt
* @param key is the key used for encryption
* @param tweak the same tweak used for encryption
*/
BigInt BOTAN_DLL fe1_decrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<byte>& tweak);

}

}


namespace Botan {

BOTAN_DLL std::vector<std::string>
list_all_readable_files_in_or_under(const std::string& dir);

}


namespace Botan {

class GHASH;

/**
* GCM Mode
*/
class BOTAN_DLL GCM_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const;

      Key_Length_Specification key_spec() const override;

      // GCM supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;
   protected:
      GCM_Mode(BlockCipher* cipher, size_t tag_size);

      const size_t BS = 16;

      const size_t m_tag_size;
      const std::string m_cipher_name;

      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<GHASH> m_ghash;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* GCM Encryption
*/
class BOTAN_DLL GCM_Encryption : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM Decryption
*/
class BOTAN_DLL GCM_Decryption : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM's GHASH
* Maybe a Transform?
*/
class BOTAN_DLL GHASH : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len);

      secure_vector<byte> nonce_hash(const byte nonce[], size_t len);

      void start(const byte nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const byte in[], size_t len);

      secure_vector<byte> final();

      Key_Length_Specification key_spec() const { return Key_Length_Specification(16); }

      void clear() override;

      std::string name() const { return "GHASH"; }
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      void gcm_multiply(secure_vector<byte>& x) const;

      void ghash_update(secure_vector<byte>& x,
                        const byte input[], size_t input_len);

      void add_final_block(secure_vector<byte>& x,
                           size_t ad_len, size_t pt_len);

      secure_vector<byte> m_H;
      secure_vector<byte> m_H_ad;
      secure_vector<byte> m_nonce;
      secure_vector<byte> m_ghash;
      size_t m_ad_len = 0, m_text_len = 0;
   };

}


namespace Botan {

using namespace gf2m_small_m;

struct polyn_gf2m
   {
   public:
      /**
      * create a zero polynomial:
      */
      polyn_gf2m( std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field );

      polyn_gf2m()
         :m_deg(-1)
         {};

      polyn_gf2m(const secure_vector<byte>& encoded, std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field );

      polyn_gf2m& operator=(const polyn_gf2m&) = default;

      bool operator==(const polyn_gf2m & other) const ;

      bool operator!=(const polyn_gf2m & other) const { return !(*this == other); };

      polyn_gf2m(polyn_gf2m&& other)
         {
         this->swap(other);
         };

      polyn_gf2m & operator=(polyn_gf2m&& other)
         {
         if(this != &other)
            {
            this->swap(other);
            }
         return *this;
         }

      void swap(polyn_gf2m& other);

      secure_vector<byte> encode() const;
      /**
      * create zero polynomial with reservation of space for a degree d polynomial
      */
      polyn_gf2m(int d, std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field);

      polyn_gf2m(polyn_gf2m const& other);
      /**
      * create zero polynomial with allocated size determined by specified degree d:
      */

      /**
      * random irreducible polynomial of degree t
      */
      polyn_gf2m(int t, RandomNumberGenerator& rng, std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field);

      std::shared_ptr<gf2m_small_m::Gf2m_Field> get_sp_field() const
         { return msp_field; };

      gf2m& operator[](size_t i) { return coeff[i]; };

      gf2m operator[](size_t i) const { return coeff[i]; }

      gf2m get_lead_coef() const { return coeff[m_deg]; }

      gf2m get_coef(u32bit i) const { return coeff[i]; }

      inline void set_coef(u32bit i, gf2m v)
         {
         coeff[i] = v;
         };

      inline void add_to_coef(u32bit i, gf2m v)
         {
         coeff[i] = coeff[i] ^ v;
         }

      std::string to_string() const;

      /** decode a polynomial from memory: **/
      polyn_gf2m(const byte* mem, u32bit mem_len, std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field);
      // remove one! ^v!
      /**
      *  create a polynomial from memory area (encoded)
      */
      polyn_gf2m(int degree, const unsigned  char* mem, u32bit mem_byte_len, std::shared_ptr<gf2m_small_m::Gf2m_Field> sp_field);

      void encode(u32bit min_numo_coeffs, byte* mem, u32bit mem_len) const;

      int get_degree() const;

      /**
      * determine the degree in a timing secure manner. the timing of this function
      * only depends on the number of allocated coefficients, not on the actual
      * degree
      */
      int calc_degree_secure() const;

      void degppf(const polyn_gf2m & g, int* p_result);

      static std::vector<polyn_gf2m> sqmod_init(const polyn_gf2m & g);

      static std::vector<polyn_gf2m> sqrt_mod_init(const polyn_gf2m & g);


      polyn_gf2m sqmod(const std::vector<polyn_gf2m> & sq, int d);
      void set_to_zero();
      gf2m eval(gf2m a);

      static std::pair<polyn_gf2m, polyn_gf2m> eea_with_coefficients(const polyn_gf2m & p,
                                                                     const polyn_gf2m & g,
                                                                     int break_deg);

      void patchup_deg_secure( u32bit trgt_deg, volatile gf2m patch_elem);

   private:

      void set_degree(int d) { m_deg = d; }

      void poly_shiftmod( const polyn_gf2m & g);
      void realloc(u32bit new_size);
      static polyn_gf2m gcd(polyn_gf2m const& p1, polyn_gf2m const& p2);

      /**
      * destructive:
      */
      static void remainder(polyn_gf2m & p, const polyn_gf2m & g);

      static polyn_gf2m gcd_aux(polyn_gf2m& p1, polyn_gf2m& p2);
   public:
      int m_deg;
      secure_vector<gf2m> coeff;
      std::shared_ptr<gf2m_small_m::Gf2m_Field> msp_field;
   };

gf2m random_code_element(unsigned code_length, RandomNumberGenerator& rng);

std::vector<polyn_gf2m> syndrome_init(polyn_gf2m const& generator, std::vector<gf2m> const& support, int n);

}


namespace Botan {

/**
* Find the roots of a polynomial over GF(2^m) using the method by Federenko
* et al.
*/
secure_vector<gf2m> find_roots_gf2m_decomp(const polyn_gf2m & polyn,
                                           u32bit code_length);

}


namespace Botan {

class BOTAN_DLL McEliece_PublicKey : public virtual Public_Key
   {
   public:

      McEliece_PublicKey(const std::vector<byte>& key_bits);

      McEliece_PublicKey(std::vector<byte> const& pub_matrix, u32bit the_t, u32bit the_code_length) :
         m_public_matrix(pub_matrix),
         m_t(the_t),
         m_code_length(the_code_length)
            {}

      McEliece_PublicKey(const McEliece_PublicKey & other);

      std::string algo_name() const { return "McEliece"; }

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const
         {
         return get_message_word_bit_length();
         };

      AlgorithmIdentifier algorithm_identifier() const;

      size_t estimated_strength() const;

      std::vector<byte> x509_subject_public_key() const;

      bool check_key(RandomNumberGenerator&, bool) const
         { return true; }

      u32bit get_t() const { return m_t; }
      u32bit get_code_length() const { return m_code_length; }
      u32bit get_message_word_bit_length() const;
      std::vector<byte> const& get_public_matrix() const { return m_public_matrix; }

      bool operator==(const McEliece_PublicKey& other) const;
      bool operator!=(const McEliece_PublicKey& other) const { return !(*this == other); }

   protected:
      McEliece_PublicKey() {}

      std::vector<byte> m_public_matrix;
      u32bit m_t;
      u32bit m_code_length;
   };

class BOTAN_DLL McEliece_PrivateKey : public virtual McEliece_PublicKey,
                                      public virtual Private_Key
   {
   public:
      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const {
      return m_Linv.size();
      };

      McEliece_PrivateKey(const secure_vector<byte>& key_bits);

      McEliece_PrivateKey(polyn_gf2m const& goppa_polyn,
                          std::vector<u32bit> const& parity_check_matrix_coeffs,
                          std::vector<polyn_gf2m> const& square_root_matrix,
                          std::vector<gf2m> const& inverse_support,
                          std::vector<byte> const& public_matrix );

      McEliece_PrivateKey(RandomNumberGenerator& rng, size_t code_length, size_t t);
      bool check_key(RandomNumberGenerator& rng, bool strong) const;

      polyn_gf2m const& get_goppa_polyn() const { return m_g; };
      std::vector<u32bit> const& get_H_coeffs() const { return m_coeffs; };
      std::vector<gf2m> const& get_Linv() const { return m_Linv; };
      std::vector<polyn_gf2m> const& get_sqrtmod() const { return m_sqrtmod; };

      inline u32bit get_dimension() const
         { return m_dimension; };

      inline u32bit get_codimension() const
         { return m_codimension; };


      secure_vector<byte> pkcs8_private_key() const;

      bool operator==(const McEliece_PrivateKey & other) const;

      bool operator!=(const McEliece_PrivateKey& other) const { return !(*this == other); };

   private:
      polyn_gf2m m_g;
      std::vector<polyn_gf2m> m_sqrtmod;
      std::vector<gf2m> m_Linv;
      std::vector<u32bit> m_coeffs;

      u32bit m_codimension;
      u32bit m_dimension;
   };

}


namespace Botan {

std::vector<byte> mceliece_encrypt(const secure_vector<byte>& cleartext,
                                   const std::vector<byte>& public_matrix,
                                   const secure_vector<gf2m> & err_pos,
                                   u32bit code_length);

secure_vector<byte> mceliece_decrypt(secure_vector<gf2m> & error_pos,
                                     const byte *ciphertext,
                                     u32bit ciphertext_len,
                                     const McEliece_PrivateKey& key);

}


namespace Botan {

/**
* The GOST 28147-89 block cipher uses a set of 4 bit Sboxes, however
* the standard does not actually define these Sboxes; they are
* considered a local configuration issue. Several different sets are
* used.
*/
class BOTAN_DLL GOST_28147_89_Params
   {
   public:
      /**
      * @param row the row
      * @param col the column
      * @return sbox entry at this row/column
      */
      byte sbox_entry(size_t row, size_t col) const;

      /**
      * @return name of this parameter set
      */
      std::string param_name() const { return name; }

      /**
      * Default GOST parameters are the ones given in GOST R 34.11 for
      * testing purposes; these sboxes are also used by Crypto++, and,
      * at least according to Wikipedia, the Central Bank of Russian
      * Federation
      * @param name of the parameter set
      */
      GOST_28147_89_Params(const std::string& name = "R3411_94_TestParam");
   private:
      const byte* sboxes;
      std::string name;
   };

/**
* GOST 28147-89
*/
class BOTAN_DLL GOST_28147_89 : public Block_Cipher_Fixed_Params<8, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();

      std::string name() const;
      BlockCipher* clone() const { return new GOST_28147_89(SBOX); }

      /**
      * @param params the sbox parameters to use
      */
      GOST_28147_89(const GOST_28147_89_Params& params);
   private:
      GOST_28147_89(const std::vector<u32bit>& other_SBOX) :
         SBOX(other_SBOX), EK(8) {}

      void key_schedule(const byte[], size_t);

      /*
      * The sbox is not secret, this is just a larger expansion of it
      * which we generate at runtime for faster execution
      */
      std::vector<u32bit> SBOX;

      secure_vector<u32bit> EK;
   };

}


namespace Botan {

/**
* GOST-34.10 Public Key
*/
class BOTAN_DLL GOST_3410_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      GOST_3410_PublicKey(const EC_Group& dom_par,
                          const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Construct from X.509 algorithm id and subject public key bits
      */
      GOST_3410_PublicKey(const AlgorithmIdentifier& alg_id,
                          const secure_vector<byte>& key_bits);

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name
      */
      std::string algo_name() const { return "GOST-34.10"; }

      AlgorithmIdentifier algorithm_identifier() const;

      std::vector<byte> x509_subject_public_key() const;

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.

      * @result the maximum number of input bits
      */
      size_t max_input_bits() const { return domain().get_order().bits(); }

      size_t message_parts() const { return 2; }

      size_t message_part_size() const
         { return domain().get_order().bytes(); }

   protected:
      GOST_3410_PublicKey() {}
   };

/**
* GOST-34.10 Private Key
*/
class BOTAN_DLL GOST_3410_PrivateKey : public GOST_3410_PublicKey,
                                       public EC_PrivateKey
   {
   public:

      GOST_3410_PrivateKey(const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key; if zero, a new random key is generated
      */
      GOST_3410_PrivateKey(RandomNumberGenerator& rng,
                           const EC_Group& domain,
                           const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      AlgorithmIdentifier pkcs8_algorithm_identifier() const
         { return EC_PublicKey::algorithm_identifier(); }
   };

}


namespace Botan {

/**
* GOST 34.11
*/
class BOTAN_DLL GOST_34_11 : public HashFunction
   {
   public:
      std::string name() const { return "GOST-R-34.11-94" ; }
      size_t output_length() const { return 32; }
      size_t hash_block_size() const { return 32; }
      HashFunction* clone() const { return new GOST_34_11; }

      void clear();

      GOST_34_11();
   private:
      void compress_n(const byte input[], size_t blocks);

      void add_data(const byte[], size_t);
      void final_result(byte[]);

      GOST_28147_89 cipher;
      secure_vector<byte> buffer, sum, hash;
      size_t position;
      u64bit count;
   };

}


namespace Botan {

/**
* MDx Hash Function Base Class
*/
class BOTAN_DLL MDx_HashFunction : public HashFunction
   {
   public:
      /**
      * @param block_length is the number of bytes per block
      * @param big_byte_endian specifies if the hash uses big-endian bytes
      * @param big_bit_endian specifies if the hash uses big-endian bits
      * @param counter_size specifies the size of the counter var in bytes
      */
      MDx_HashFunction(size_t block_length,
                       bool big_byte_endian,
                       bool big_bit_endian,
                       size_t counter_size = 8);

      size_t hash_block_size() const { return buffer.size(); }
   protected:
      void add_data(const byte input[], size_t length);
      void final_result(byte output[]);

      /**
      * Run the hash's compression function over a set of blocks
      * @param blocks the input
      * @param block_n the number of blocks
      */
      virtual void compress_n(const byte blocks[], size_t block_n) = 0;

      void clear();

      /**
      * Copy the output to the buffer
      * @param buffer to put the output into
      */
      virtual void copy_out(byte buffer[]) = 0;

      /**
      * Write the count, if used, to this spot
      * @param out where to write the counter to
      */
      virtual void write_count(byte out[]);
   private:
      secure_vector<byte> buffer;
      u64bit count;
      size_t position;

      const bool BIG_BYTE_ENDIAN, BIG_BIT_ENDIAN;
      const size_t COUNT_SIZE;
   };

}


namespace Botan {

/**
* HAS-160, a Korean hash function standardized in
* TTAS.KO-12.0011/R1. Used in conjuction with KCDSA
*/
class BOTAN_DLL HAS_160 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "HAS-160"; }
      size_t output_length() const { return 20; }
      HashFunction* clone() const { return new HAS_160; }

      void clear();

      HAS_160() : MDx_HashFunction(64, false, true), X(20), digest(5)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u32bit> X, digest;
   };

}


namespace Botan {

/**
* Return the PKCS #1 hash identifier
* @see RFC 3447 section 9.2
* @param hash_name the name of the hash function
* @return byte sequence identifying the hash
* @throw Invalid_Argument if the hash has no known PKCS #1 hash id
*/
BOTAN_DLL std::vector<byte> pkcs_hash_id(const std::string& hash_name);

/**
* Return the IEEE 1363 hash identifier
* @param hash_name the name of the hash function
* @return byte code identifying the hash, or 0 if not known
*/
BOTAN_DLL byte ieee1363_hash_id(const std::string& hash_name);

}


namespace Botan {

/**
* Perform hex encoding
* @param output an array of at least input_length*2 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
*/
void BOTAN_DLL hex_encode(char output[],
                          const byte input[],
                          size_t input_length,
                          bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
std::string BOTAN_DLL hex_encode(const byte input[],
                                 size_t input_length,
                                 bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
template<typename Alloc>
std::string hex_encode(const std::vector<byte, Alloc>& input,
                       bool uppercase = true)
   {
   return hex_encode(&input[0], input.size(), uppercase);
   }

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            size_t& input_consumed,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const std::string& input,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const char input[],
           size_t input_length,
           bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const std::string& input,
           bool ignore_ws = true);


/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const char input[],
                  size_t input_length,
                  bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const std::string& input,
                  bool ignore_ws = true);

}


namespace Botan {

/**
* HKDF, see @rfc 5869 for details
* This is only the expansion portion of HKDF
*/
class BOTAN_DLL HKDF : public KDF
   {
   public:
      HKDF(MessageAuthenticationCode* prf) : m_prf(prf) {}

      static HKDF* make(const Spec& spec);

      KDF* clone() const { return new HKDF(m_prf->clone()); }

      std::string name() const { return "HKDF(" + m_prf->name() + ")"; }

      size_t kdf(byte out[], size_t out_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}


namespace Botan {

/**
* HMAC
*/
class BOTAN_DLL HMAC : public MessageAuthenticationCode
   {
   public:
      void clear();
      std::string name() const;
      MessageAuthenticationCode* clone() const;

      size_t output_length() const { return m_hash->output_length(); }

      Key_Length_Specification key_spec() const
         {
         // Absurd max length here is to support PBKDF2
         return Key_Length_Specification(0, 512);
         }

      /**
      * @param hash the hash to use for HMACing
      */
      HMAC(HashFunction* hash);

      static HMAC* make(const Spec& spec);

      HMAC(const HMAC&) = delete;
      HMAC& operator=(const HMAC&) = delete;
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<byte> m_ikey, m_okey;
   };

}


namespace Botan {

/**
* HMAC_DRBG (SP800-90A)
*/
class BOTAN_DLL HMAC_DRBG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t buf_len);
      bool is_seeded() const;
      void clear();
      std::string name() const;

      void reseed(size_t poll_bits);

      void add_entropy(const byte input[], size_t input_len);

      /**
      * @param mac the underlying mac function (eg HMAC(SHA-512))
      * @param underlying_rng RNG used generating inputs (eg HMAC_RNG)
      */
      HMAC_DRBG(MessageAuthenticationCode* mac,
                RandomNumberGenerator* underlying_rng);

   private:
      void update(const byte input[], size_t input_len);

      std::unique_ptr<MessageAuthenticationCode> m_mac;
      std::unique_ptr<RandomNumberGenerator> m_prng;

      secure_vector<byte> m_V;
      size_t m_reseed_counter;
   };

}


namespace Botan {

/**
* HMAC_RNG - based on the design described in "On Extract-then-Expand
* Key Derivation Functions and an HMAC-based KDF" by Hugo Krawczyk
* (henceforce, 'E-t-E')
*
* However it actually can be parameterized with any two MAC functions,
* not restricted to HMAC (this variation is also described in
* Krawczyk's paper), for instance one could use HMAC(SHA-512) as the
* extractor and CMAC(AES-256) as the PRF.
*/
class BOTAN_DLL HMAC_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t len);
      bool is_seeded() const;
      void clear();
      std::string name() const;

      void reseed(size_t poll_bits);
      void add_entropy(const byte[], size_t);

      /**
      * @param extractor a MAC used for extracting the entropy
      * @param prf a MAC used as a PRF using HKDF construction
      */
      HMAC_RNG(MessageAuthenticationCode* extractor,
               MessageAuthenticationCode* prf);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_extractor;
      std::unique_ptr<MessageAuthenticationCode> m_prf;

      size_t m_collected_entropy_estimate = 0;
      size_t m_output_since_reseed = 0;

      secure_vector<byte> m_K;
      u32bit m_counter = 0;
   };

}


namespace Botan {

namespace HTTP {

struct Response
   {
   public:
      Response() : m_status_code(0), m_status_message("Uninitialized") {}

      Response(unsigned int status_code, const std::string& status_message,
               const std::vector<byte>& body,
               const std::map<std::string, std::string>& headers) :
         m_status_code(status_code),
         m_status_message(status_message),
         m_body(body),
         m_headers(headers) {}

      unsigned int status_code() const { return m_status_code; }

      const std::vector<byte>& body() const { return m_body; }

      const std::map<std::string, std::string>& headers() const { return m_headers; }

      std::string status_message() const { return m_status_message; }

      void throw_unless_ok()
         {
         if(status_code() != 200)
            throw std::runtime_error("HTTP error: " + status_message());
         }

   private:
      unsigned int m_status_code;
      std::string m_status_message;
      std::vector<byte> m_body;
      std::map<std::string, std::string> m_headers;
   };

BOTAN_DLL std::ostream& operator<<(std::ostream& o, const Response& resp);

typedef std::function<std::string (const std::string&, const std::string&)> http_exch_fn;

#if defined(BOTAN_HAS_BOOST_ASIO)
std::string BOTAN_DLL http_transact_asio(const std::string& hostname,
                                         const std::string& message);
#endif

std::string BOTAN_DLL http_transact_fail(const std::string& hostname,
                                         const std::string& message);


BOTAN_DLL Response http_sync(http_exch_fn fn,
                             const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response http_sync(const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response GET_sync(const std::string& url,
                            size_t allowable_redirects = 1);

BOTAN_DLL Response POST_sync(const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects = 1);

std::future<Response> BOTAN_DLL GET_async(const std::string& url,
                                                    size_t allowable_redirects = 1);

BOTAN_DLL std::string url_encode(const std::string& url);

}

}


namespace Botan {

/**
* IDEA
*/
class BOTAN_DLL IDEA : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "IDEA"; }
      BlockCipher* clone() const { return new IDEA; }
   protected:
      /**
      * @return const reference to encryption subkeys
      */
      const secure_vector<u16bit>& get_EK() const { return EK; }

      /**
      * @return const reference to decryption subkeys
      */
      const secure_vector<u16bit>& get_DK() const { return DK; }

   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK, DK;
   };

}


namespace Botan {

/**
* IDEA in SSE2
*/
class BOTAN_DLL IDEA_SSE2 : public IDEA
   {
   public:
      size_t parallelism() const { return 8; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new IDEA_SSE2; }
   };

}


namespace Botan {

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PublicKey : public virtual Public_Key
   {
   public:
      IF_Scheme_PublicKey(const AlgorithmIdentifier& alg_id,
                          const secure_vector<byte>& key_bits);

      IF_Scheme_PublicKey(const BigInt& n, const BigInt& e) :
         n(n), e(e) {}

      bool check_key(RandomNumberGenerator& rng, bool) const;

      AlgorithmIdentifier algorithm_identifier() const;

      std::vector<byte> x509_subject_public_key() const;

      /**
      * @return public modulus
      */
      const BigInt& get_n() const { return n; }

      /**
      * @return public exponent
      */
      const BigInt& get_e() const { return e; }

      size_t max_input_bits() const { return (n.bits() - 1); }

      size_t estimated_strength() const override;

   protected:
      IF_Scheme_PublicKey() {}

      BigInt n, e;
   };

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PrivateKey : public virtual IF_Scheme_PublicKey,
                                       public virtual Private_Key
   {
   public:

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const BigInt& prime1, const BigInt& prime2,
                           const BigInt& exp, const BigInt& d_exp,
                           const BigInt& mod);

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits);

      bool check_key(RandomNumberGenerator& rng, bool) const;

      /**
      * Get the first prime p.
      * @return prime p
      */
      const BigInt& get_p() const { return p; }

      /**
      * Get the second prime q.
      * @return prime q
      */
      const BigInt& get_q() const { return q; }

      /**
      * Get d with exp * d = 1 mod (p - 1, q - 1).
      * @return d
      */
      const BigInt& get_d() const { return d; }

      const BigInt& get_c() const { return c; }
      const BigInt& get_d1() const { return d1; }
      const BigInt& get_d2() const { return d2; }

      secure_vector<byte> pkcs8_private_key() const;

   protected:
      IF_Scheme_PrivateKey() {}

      BigInt d, p, q, d1, d2, c;
   };

}


namespace Botan {

/*
* Previously botan had state whose lifetime had to be explicitly
* managed by the application. As of 1.11.14 this is no longer the
* case, and this class is no longer needed and kept only for backwards
* compatability.
*/
class BOTAN_DLL LibraryInitializer
   {
   public:
      LibraryInitializer(const std::string& = "") {}
      ~LibraryInitializer() {}
      static void initialize(const std::string& = "") {}
      static void deinitialize() {}
   };

}


namespace Botan {

/**
* KASUMI, the block cipher used in 3G telephony
*/
class BOTAN_DLL KASUMI : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "KASUMI"; }
      BlockCipher* clone() const { return new KASUMI; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK;
   };

}


namespace Botan {

/**
* KDF1, from IEEE 1363
*/
class BOTAN_DLL KDF1 : public KDF
   {
   public:
      std::string name() const override { return "KDF1(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF1(m_hash->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      KDF1(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* KDF2, from IEEE 1363
*/
class BOTAN_DLL KDF2 : public KDF
   {
   public:
      std::string name() const override { return "KDF2(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF2(m_hash->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      KDF2(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* Keccak[1600], a SHA-3 candidate
*/
class BOTAN_DLL Keccak_1600 : public HashFunction
   {
   public:

      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      Keccak_1600(size_t output_bits = 512);

      size_t hash_block_size() const { return bitrate / 8; }
      size_t output_length() const { return output_bits / 8; }

      HashFunction* clone() const;
      std::string name() const;
      void clear();
   private:
      void add_data(const byte input[], size_t length);
      void final_result(byte out[]);

      size_t output_bits, bitrate;
      secure_vector<u64bit> S;
      size_t S_pos;
   };

}


namespace Botan {

namespace KeyPair {

/**
* Tests whether the key is consistent for encryption; whether
* encrypting and then decrypting gives to the original plaintext.
* @param rng the rng to use
* @param key the key to test
* @param padding the encryption padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
encryption_consistency_check(RandomNumberGenerator& rng,
                             const Private_Key& key,
                             const std::string& padding);

/**
* Tests whether the key is consistent for signatures; whether a
* signature can be created and then verified
* @param rng the rng to use
* @param key the key to test
* @param padding the signature padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
signature_consistency_check(RandomNumberGenerator& rng,
                            const Private_Key& key,
                            const std::string& padding);

}

}


namespace Botan {

/**
* Lion is a block cipher construction designed by Ross Anderson and
* Eli Biham, described in "Two Practical and Provably Secure Block
* Ciphers: BEAR and LION". It has a variable block size and is
* designed to encrypt very large blocks (up to a megabyte)

* http://www.cl.cam.ac.uk/~rja14/Papers/bear-lion.pdf
*/
class BOTAN_DLL Lion : public BlockCipher
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      size_t block_size() const override { return m_block_size; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(2, 2*m_hash->output_length(), 2);
         }

      void clear() override;
      std::string name() const override;
      BlockCipher* clone() const override;

      /**
      * @param hash the hash to use internally
      * @param cipher the stream cipher to use internally
      * @param block_size the size of the block to use
      */
      Lion(HashFunction* hash,
           StreamCipher* cipher,
           size_t block_size);
   private:
      void key_schedule(const byte[], size_t);

      size_t left_size() const { return m_hash->output_length(); }
      size_t right_size() const { return m_block_size - left_size(); }

      const size_t m_block_size;
      std::unique_ptr<HashFunction> m_hash;
      std::unique_ptr<StreamCipher> m_cipher;
      secure_vector<byte> m_key1, m_key2;
   };

}


#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)

#define BOTAN_ENDIAN_N2B(x) (x)
#define BOTAN_ENDIAN_B2N(x) (x)

#define BOTAN_ENDIAN_N2L(x) reverse_bytes(x)
#define BOTAN_ENDIAN_L2N(x) reverse_bytes(x)

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)

#define BOTAN_ENDIAN_N2L(x) (x)
#define BOTAN_ENDIAN_L2N(x) (x)

#define BOTAN_ENDIAN_N2B(x) reverse_bytes(x)
#define BOTAN_ENDIAN_B2N(x) reverse_bytes(x)

#endif

#endif

namespace Botan {

/**
* Make a u16bit from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline u16bit make_u16bit(byte i0, byte i1)
   {
   return ((static_cast<u16bit>(i0) << 8) | i1);
   }

/**
* Make a u32bit from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline u32bit make_u32bit(byte i0, byte i1, byte i2, byte i3)
   {
   return ((static_cast<u32bit>(i0) << 24) |
           (static_cast<u32bit>(i1) << 16) |
           (static_cast<u32bit>(i2) <<  8) |
           (static_cast<u32bit>(i3)));
   }

/**
* Make a u32bit from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline u64bit make_u64bit(byte i0, byte i1, byte i2, byte i3,
                          byte i4, byte i5, byte i6, byte i7)
    {
   return ((static_cast<u64bit>(i0) << 56) |
           (static_cast<u64bit>(i1) << 48) |
           (static_cast<u64bit>(i2) << 40) |
           (static_cast<u64bit>(i3) << 32) |
           (static_cast<u64bit>(i4) << 24) |
           (static_cast<u64bit>(i5) << 16) |
           (static_cast<u64bit>(i6) <<  8) |
           (static_cast<u64bit>(i7)));
    }

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
template<typename T>
inline T load_be(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[i];
   return out;
   }

/**
* Load a little-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a litte-endian value
*/
template<typename T>
inline T load_le(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[sizeof(T)-1-i];
   return out;
   }

/**
* Load a big-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a big-endian value
*/
template<>
inline u16bit load_be<u16bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u16bit*>(in) + off));
#else
   in += off * sizeof(u16bit);
   return make_u16bit(in[0], in[1]);
#endif
   }

/**
* Load a little-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a little-endian value
*/
template<>
inline u16bit load_le<u16bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u16bit*>(in) + off));
#else
   in += off * sizeof(u16bit);
   return make_u16bit(in[1], in[0]);
#endif
   }

/**
* Load a big-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a big-endian value
*/
template<>
inline u32bit load_be<u32bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u32bit*>(in) + off));
#else
   in += off * sizeof(u32bit);
   return make_u32bit(in[0], in[1], in[2], in[3]);
#endif
   }

/**
* Load a little-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a little-endian value
*/
template<>
inline u32bit load_le<u32bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u32bit*>(in) + off));
#else
   in += off * sizeof(u32bit);
   return make_u32bit(in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load a big-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a big-endian value
*/
template<>
inline u64bit load_be<u64bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u64bit*>(in) + off));
#else
   in += off * sizeof(u64bit);
   return make_u64bit(in[0], in[1], in[2], in[3],
                      in[4], in[5], in[6], in[7]);
#endif
   }

/**
* Load a little-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a little-endian value
*/
template<>
inline u64bit load_le<u64bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u64bit*>(in) + off));
#else
   in += off * sizeof(u64bit);
   return make_u64bit(in[7], in[6], in[5], in[4],
                      in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load two little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_le(const byte in[], T& x0, T& x1)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   }

/**
* Load four little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   }

/**
* Load eight little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   x4 = load_le<T>(in, 4);
   x5 = load_le<T>(in, 5);
   x6 = load_le<T>(in, 6);
   x7 = load_le<T>(in, 7);
   }

/**
* Load a variable number of little-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_le(T out[],
                    const byte in[],
                    size_t count)
   {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
   std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
   const size_t blocks = count - (count % 4);
   const size_t left = count - blocks;

   for(size_t i = 0; i != blocks; i += 4)
      bswap_4(out + i);

   for(size_t i = 0; i != left; ++i)
      out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
   for(size_t i = 0; i != count; ++i)
      out[i] = load_le<T>(in, i);
#endif
   }

/**
* Load two big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_be(const byte in[], T& x0, T& x1)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   }

/**
* Load four big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   }

/**
* Load eight big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   x4 = load_be<T>(in, 4);
   x5 = load_be<T>(in, 5);
   x6 = load_be<T>(in, 6);
   x7 = load_be<T>(in, 7);
   }

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_be(T out[],
                    const byte in[],
                    size_t count)
   {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
   std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
   const size_t blocks = count - (count % 4);
   const size_t left = count - blocks;

   for(size_t i = 0; i != blocks; i += 4)
      bswap_4(out + i);

   for(size_t i = 0; i != left; ++i)
      out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
   for(size_t i = 0; i != count; ++i)
      out[i] = load_be<T>(in, i);
#endif
   }

/**
* Store a big-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_be(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u16bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
#endif
   }

/**
* Store a little-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_le(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u16bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(1, in);
   out[1] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_be(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u32bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
#endif
   }

/**
* Store a little-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_le(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u32bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(3, in);
   out[1] = get_byte(2, in);
   out[2] = get_byte(1, in);
   out[3] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_be(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u64bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
   out[4] = get_byte(4, in);
   out[5] = get_byte(5, in);
   out[6] = get_byte(6, in);
   out[7] = get_byte(7, in);
#endif
   }

/**
* Store a little-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_le(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u64bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(7, in);
   out[1] = get_byte(6, in);
   out[2] = get_byte(5, in);
   out[3] = get_byte(4, in);
   out[4] = get_byte(3, in);
   out[5] = get_byte(2, in);
   out[6] = get_byte(1, in);
   out[7] = get_byte(0, in);
#endif
   }

/**
* Store two little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   }

/**
* Store two big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   }

/**
* Store four little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   }

/**
* Store four big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   }

/**
* Store eight little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   store_le(x4, out + (4 * sizeof(T)));
   store_le(x5, out + (5 * sizeof(T)));
   store_le(x6, out + (6 * sizeof(T)));
   store_le(x7, out + (7 * sizeof(T)));
   }

/**
* Store eight big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   store_be(x4, out + (4 * sizeof(T)));
   store_be(x5, out + (5 * sizeof(T)));
   store_be(x6, out + (6 * sizeof(T)));
   store_be(x7, out + (7 * sizeof(T)));
   }

template<typename T>
void copy_out_be(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_be(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(i%8, in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_be(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_be(out, out_bytes, &in[0]);
   }

template<typename T>
void copy_out_le(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_le(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(sizeof(T) - 1 - (i % 8), in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_le(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_le(out, out_bytes, &in[0]);
   }

}


namespace Botan {

/**
* MARS, IBM's candidate for AES
*/
class BOTAN_DLL MARS : public Block_Cipher_Fixed_Params<16, 16, 32, 4>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "MARS"; }
      BlockCipher* clone() const { return new MARS; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK;
   };

}


#define MASK_LOG2_BYTE ((1 << 3) - 1)
#define _BITP_TO_BYTEP(__bit_pos) (__bit_pos >> 3)
#define _BITP_TO_BYTEOFFS(__bit_pos) (__bit_pos & MASK_LOG2_BYTE)

namespace Botan {

secure_vector<gf2m> BOTAN_DLL create_random_error_positions(unsigned code_length, unsigned error_weight, RandomNumberGenerator& rng);

class mceliece_message_parts
   {
   public:

      mceliece_message_parts(const secure_vector<gf2m>& err_pos, const byte* message, u32bit message_length, u32bit code_length) :
         m_error_vector(error_vector_from_error_positions(&err_pos[0], err_pos.size(), code_length)),
         m_code_length(code_length)
         {
         m_message_word.resize(message_length);
         copy_mem(&m_message_word[0], message, message_length);
         }

      mceliece_message_parts(const secure_vector<gf2m>& err_pos, const secure_vector<byte>& message, unsigned code_length) :
         m_error_vector(error_vector_from_error_positions(&err_pos[0], err_pos.size(), code_length)),
         m_message_word(message),
         m_code_length(code_length)
         {}

      static secure_vector<byte> error_vector_from_error_positions(const gf2m* err_pos, size_t err_pos_len, size_t code_length)
         {
         secure_vector<byte> result((code_length+7)/8);
         for(unsigned i = 0; i < err_pos_len; i++)
            {
            u16bit pos = err_pos[i];
            u32bit byte_pos = _BITP_TO_BYTEP(pos);
            if(byte_pos > result.size())
               {
               throw Invalid_Argument("error position larger than code size");
               }
            result[byte_pos] |= (1 << _BITP_TO_BYTEOFFS(pos));
            }
         return result;
         }

      mceliece_message_parts(const byte* message_concat_errors, size_t message_concat_errors_len, unsigned code_length) :
         m_code_length(code_length)
         {
         size_t err_vec_len = (code_length+7)/8;
         if(message_concat_errors_len < err_vec_len )
            {
            throw Invalid_Argument("cannot split McEliece message parts");
            }
         size_t err_vec_start_pos = message_concat_errors_len - err_vec_len;
         m_message_word = secure_vector<byte>(err_vec_start_pos );
         copy_mem(&m_message_word[0], &message_concat_errors[0], err_vec_start_pos);
         m_error_vector = secure_vector<byte>(err_vec_len );
         copy_mem(&m_error_vector[0],  &message_concat_errors[err_vec_start_pos], err_vec_len);
         }

      secure_vector<byte> get_concat() const
         {
         secure_vector<byte> result(m_error_vector.size() + m_message_word.size());
         copy_mem(&result[0], &m_message_word[0], m_message_word.size());
         copy_mem(&result[m_message_word.size()], &m_error_vector[0], m_error_vector.size());
         return result;
         }

      secure_vector<gf2m> get_error_positions() const
         {
         secure_vector<gf2m> result;
         for(unsigned i = 0; i < m_code_length; i++)
            {
            if(i >= m_code_length)
               {
               throw Invalid_Argument("index out of range in get_error_positions()");
               }
            if((m_error_vector[_BITP_TO_BYTEP(i)] >> _BITP_TO_BYTEOFFS(i)) & 1)
               {
               result.push_back(i);
               }
            }
         return result;
         }

      secure_vector<byte> get_error_vector() const { return m_error_vector; }
      secure_vector<byte> get_message_word() const { return m_message_word; }
   private:
      secure_vector<byte> m_error_vector;
      secure_vector<byte> m_message_word;
      unsigned m_code_length;
   };

class BOTAN_DLL McEliece_Private_Operation : public PK_Ops::Decryption
   {
   public:
      McEliece_Private_Operation(const McEliece_PrivateKey& mce_key);

      size_t max_input_bits() const { return m_priv_key.max_input_bits();  }

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len);

      McEliece_PrivateKey const& get_key() const { return m_priv_key; }

   private:
      const McEliece_PrivateKey m_priv_key;
   };

class BOTAN_DLL McEliece_Public_Operation : public PK_Ops::Encryption
   {
   public:
      McEliece_Public_Operation(const McEliece_PublicKey& public_key, u32bit code_length);

      size_t max_input_bits() const { return m_pub_key.max_input_bits(); }
      secure_vector<byte> encrypt(const byte msg[], size_t msg_len, RandomNumberGenerator&);

      McEliece_PublicKey const& get_key() const { return m_pub_key; }

   private:
      McEliece_PublicKey m_pub_key;
      u32bit m_code_length;
   };

/**
* Estimate work factor for McEliece
* @return estimated security level for these key parameters
*/
BOTAN_DLL size_t mceliece_work_factor(size_t code_size, size_t k, size_t t);

}



namespace Botan {

class BOTAN_DLL McEliece_KEM_Encryptor
   {
   public:
      McEliece_KEM_Encryptor(const McEliece_PublicKey& public_key);

      /**
      * returns the pair (mceliece ciphertext, symmetric key)
      */
      std::pair<secure_vector<byte>, secure_vector<byte>> encrypt(RandomNumberGenerator& rng);

   private:
      McEliece_Public_Operation m_raw_pub_op;
   };

class BOTAN_DLL McEliece_KEM_Decryptor
   {
    public:
      McEliece_KEM_Decryptor(const McEliece_PrivateKey& mce_key);

      /**
      * returns the derived 512-bit symmetric key
      */
      secure_vector<Botan::byte> decrypt(const byte msg[], size_t msg_len);

      /**
      * returns the derived 512-bit symmetric key
      */
      template<typename Alloc>
      secure_vector<Botan::byte> decrypt_vec(const std::vector<byte, Alloc>& v)
         {
         return decrypt(&v[0], v.size());

         }
   private:
      McEliece_Private_Operation m_raw_priv_op;
  };
}


namespace Botan {

class McEliece_PublicKey;
class McEliece_PrivateKey;

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and encrypt/authenticate the
* plaintext and AD using AES-256 in OCB mode.
*/
secure_vector<byte>
BOTAN_DLL mceies_encrypt(const McEliece_PublicKey& pubkey,
                         const secure_vector<byte>& pt,
                         byte ad[], size_t ad_len,
                         RandomNumberGenerator& rng);

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and decrypt/authenticate the
* ciphertext and AD using AES-256 in OCB mode.
*/
secure_vector<byte>
BOTAN_DLL mceies_decrypt(const McEliece_PrivateKey& privkey,
                         const secure_vector<byte>& ct,
                         byte ad[], size_t ad_len);


}


namespace Botan {

/**
* MD2
*/
class BOTAN_DLL MD2 : public HashFunction
   {
   public:
      std::string name() const { return "MD2"; }
      size_t output_length() const { return 16; }
      size_t hash_block_size() const { return 16; }
      HashFunction* clone() const { return new MD2; }

      void clear();

      MD2() : X(48), checksum(16), buffer(16)
         { clear(); }
   private:
      void add_data(const byte[], size_t);
      void hash(const byte[]);
      void final_result(byte[]);

      secure_vector<byte> X, checksum, buffer;
      size_t position;
   };

}


namespace Botan {

/**
* MD4
*/
class BOTAN_DLL MD4 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "MD4"; }
      size_t output_length() const { return 16; }
      HashFunction* clone() const { return new MD4; }

      void clear();

      MD4() : MDx_HashFunction(64, false, true), M(16), digest(4)
         { clear(); }
   protected:
      void compress_n(const byte input[], size_t blocks);
      void copy_out(byte[]);

      /**
      * The message buffer, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> M;

      /**
      * The digest value, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> digest;
   };

}


namespace Botan {

/**
* MD5
*/
class BOTAN_DLL MD5 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "MD5"; }
      size_t output_length() const { return 16; }
      HashFunction* clone() const { return new MD5; }

      void clear();

      MD5() : MDx_HashFunction(64, false, true), M(16), digest(4)
         { clear(); }
   protected:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      /**
      * The message buffer, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> M;

      /**
      * The digest value, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> digest;
   };

}


namespace Botan {

/**
* MGF1 from PKCS #1 v2.0
*/
void mgf1_mask(HashFunction& hash,
               const byte in[], size_t in_len,
               byte out[], size_t out_len);

}


namespace Botan {

/**
* MISTY1 with 8 rounds
*/
class BOTAN_DLL MISTY1 : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "MISTY1"; }
      BlockCipher* clone() const { return new MISTY1; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK, DK;
   };

}


namespace Botan {

/**
* Noekeon
*/
class BOTAN_DLL Noekeon : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Noekeon"; }
      BlockCipher* clone() const { return new Noekeon; }
   protected:
      /**
      * The Noekeon round constants
      */
      static const byte RC[17];

      /**
      * @return const reference to encryption subkeys
      */
      const secure_vector<u32bit>& get_EK() const { return EK; }

      /**
      * @return const reference to decryption subkeys
      */
      const secure_vector<u32bit>& get_DK() const { return DK; }

   private:
      void key_schedule(const byte[], size_t);
      secure_vector<u32bit> EK, DK;
   };

}


namespace Botan {

/**
* Noekeon implementation using SIMD operations
*/
class BOTAN_DLL Noekeon_SIMD : public Noekeon
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new Noekeon_SIMD; }
   };

}


namespace Botan {

/**
* Nyberg-Rueppel Public Key
*/
class BOTAN_DLL NR_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "NR"; }

      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_57; }

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return group_q().bytes(); }
      size_t max_input_bits() const { return (group_q().bits() - 1); }

      NR_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits);

      NR_PublicKey(const DL_Group& group, const BigInt& pub_key);
   protected:
      NR_PublicKey() {}
   };

/**
* Nyberg-Rueppel Private Key
*/
class BOTAN_DLL NR_PrivateKey : public NR_PublicKey,
                                public virtual DL_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool strong) const;

      NR_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits,
                    RandomNumberGenerator& rng);

      NR_PrivateKey(RandomNumberGenerator& rng,
                    const DL_Group& group,
                    const BigInt& x = 0);
   };

}


namespace Botan {

/**
* OAEP (called EME1 in IEEE 1363 and in earlier versions of the library)
*/
class BOTAN_DLL OAEP : public EME
   {
   public:
      size_t maximum_input_size(size_t) const;

      static OAEP* make(const Spec& spec);

      /**
      * @param hash object to use for hashing (takes ownership)
      * @param P an optional label. Normally empty.
      */
      OAEP(HashFunction* hash, const std::string& P = "");
   private:
      secure_vector<byte> pad(const byte[], size_t, size_t,
                             RandomNumberGenerator&) const;
      secure_vector<byte> unpad(const byte[], size_t, size_t) const;

      secure_vector<byte> m_Phash;
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

class L_computer;

/**
* OCB Mode (base class for OCB_Encryption and OCB_Decryption). Note
* that OCB is patented, but is freely licensed in some circumstances.
*
* @see "The OCB Authenticated-Encryption Algorithm" internet draft
        http://tools.ietf.org/html/draft-irtf-cfrg-ocb-03
* @see Free Licenses http://www.cs.ucdavis.edu/~rogaway/ocb/license.htm
* @see OCB home page http://www.cs.ucdavis.edu/~rogaway/ocb
*/
class BOTAN_DLL OCB_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      ~OCB_Mode();
   protected:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Mode(BlockCipher* cipher, size_t tag_size);

      size_t BS() const { return m_BS; }

      // fixme make these private
      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<L_computer> m_L;

      size_t m_BS;
      size_t m_block_index = 0;

      secure_vector<byte> m_checksum;
      secure_vector<byte> m_offset;
      secure_vector<byte> m_ad_hash;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      secure_vector<byte> update_nonce(const byte nonce[], size_t nonce_len);

      size_t m_tag_size = 0;
      secure_vector<byte> m_last_nonce;
      secure_vector<byte> m_stretch;
   };

class BOTAN_DLL OCB_Encryption : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   private:
      void encrypt(byte input[], size_t blocks);
   };

class BOTAN_DLL OCB_Decryption : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   private:
      void decrypt(byte input[], size_t blocks);
   };

}


namespace Botan {

namespace OCSP {

class BOTAN_DLL CertID : public ASN1_Object
   {
   public:
      CertID() {}

      CertID(const X509_Certificate& issuer,
             const X509_Certificate& subject);

      bool is_id_for(const X509_Certificate& issuer,
                     const X509_Certificate& subject) const;

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;
   private:
      std::vector<byte> extract_key_bitstr(const X509_Certificate& cert) const;

      AlgorithmIdentifier m_hash_id;
      std::vector<byte> m_issuer_dn_hash;
      std::vector<byte> m_issuer_key_hash;
      BigInt m_subject_serial;
   };

class BOTAN_DLL SingleResponse : public ASN1_Object
   {
   public:
      const CertID& certid() const { return m_certid; }

      size_t cert_status() const { return m_cert_status; }

      X509_Time this_update() const { return m_thisupdate; }

      X509_Time next_update() const { return m_nextupdate; }

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;
   private:
      CertID m_certid;
      size_t m_cert_status = 2; // unknown
      X509_Time m_thisupdate;
      X509_Time m_nextupdate;
   };

}

}


namespace Botan {

class Certificate_Store;

namespace OCSP {

class BOTAN_DLL Request
   {
   public:
      Request(const X509_Certificate& issuer_cert,
              const X509_Certificate& subject_cert) :
         m_issuer(issuer_cert),
         m_subject(subject_cert)
         {}

      std::vector<byte> BER_encode() const;

      std::string base64_encode() const;

      const X509_Certificate& issuer() const { return m_issuer; }

      const X509_Certificate& subject() const { return m_subject; }
   private:
      X509_Certificate m_issuer, m_subject;
   };

class BOTAN_DLL Response
   {
   public:
      Response() {}

      Response(const Certificate_Store& trusted_roots,
               const std::vector<byte>& response);

      Certificate_Status_Code status_for(const X509_Certificate& issuer,
                                               const X509_Certificate& subject) const;

   private:
      std::vector<SingleResponse> m_responses;
   };

BOTAN_DLL Response online_check(const X509_Certificate& issuer,
                                const X509_Certificate& subject,
                                const Certificate_Store* trusted_roots);

}

}


namespace Botan {

/**
* Output Feedback Mode
*/
class BOTAN_DLL OFB : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const
         {
         return m_cipher->key_spec();
         }

      std::string name() const;

      OFB* clone() const
         { return new OFB(m_cipher->clone()); }

      void clear();

      static OFB* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      OFB(BlockCipher* cipher);
   private:
      void key_schedule(const byte key[], size_t key_len);

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_buffer;
      size_t m_buf_pos;
   };

}


namespace Botan {

namespace OIDS {

/**
* Register an OID to string mapping.
* @param oid the oid to register
* @param name the name to be associated with the oid
*/
BOTAN_DLL void add_oid(const OID& oid, const std::string& name);

BOTAN_DLL void add_oid2str(const OID& oid, const std::string& name);
BOTAN_DLL void add_str2oid(const OID& oid, const std::string& name);

BOTAN_DLL void add_oidstr(const char* oidstr, const char* name);

/**
* See if an OID exists in the internal table.
* @param oid the oid to check for
* @return true if the oid is registered
*/
BOTAN_DLL bool have_oid(const std::string& oid);

/**
* Resolve an OID
* @param oid the OID to look up
* @return name associated with this OID
*/
BOTAN_DLL std::string lookup(const OID& oid);

/**
* Find the OID to a name. The lookup will be performed in the
* general OID section of the configuration.
* @param name the name to resolve
* @return OID associated with the specified name
*/
BOTAN_DLL OID lookup(const std::string& name);

/**
* Tests whether the specified OID stands for the specified name.
* @param oid the OID to check
* @param name the name to check
* @return true if the specified OID stands for the specified name
*/
BOTAN_DLL bool name_of(const OID& oid, const std::string& name);

BOTAN_DLL const char* default_oid_list();

}

}


namespace Botan {

/**
* @param input the input data
* @param length length of input in bytes
* @param label the human-readable label
* @param headers a set of key/value pairs included in the header
*/
BOTAN_DLL std::string PGP_encode(
   const byte input[],
   size_t length,
   const std::string& label,
   const std::map<std::string, std::string>& headers);

/**
* @param input the input data
* @param length length of input in bytes
* @param label the human-readable label
*/
BOTAN_DLL std::string PGP_encode(
   const byte input[],
   size_t length,
   const std::string& label);

/**
* @param source the input source
* @param label is set to the human-readable label
* @param headers is set to any headers
* @return decoded output as raw binary
*/
BOTAN_DLL secure_vector<byte> PGP_decode(
   DataSource& source,
   std::string& label,
   std::map<std::string, std::string>& headers);

/**
* @param source the input source
* @param label is set to the human-readable label
* @return decoded output as raw binary
*/
BOTAN_DLL secure_vector<byte> PGP_decode(
   DataSource& source,
   std::string& label);

}


namespace Botan {

/**
* Rivest's Package Tranform
* @param rng the random number generator to use
* @param cipher the block cipher to use
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len + cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_package(RandomNumberGenerator& rng,
                            BlockCipher* cipher,
                            const byte input[], size_t input_len,
                            byte output[]);

/**
* Rivest's Package Tranform (Inversion)
* @param cipher the block cipher to use
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len - cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_unpackage(BlockCipher* cipher,
                              const byte input[], size_t input_len,
                              byte output[]);

}


namespace Botan {

/**
* Parallel Hashes
*/
class BOTAN_DLL Parallel : public HashFunction
   {
   public:
      void clear();
      std::string name() const;
      HashFunction* clone() const;

      size_t output_length() const;

      /**
      * @param hashes a set of hashes to compute in parallel
      */
      Parallel(const std::vector<HashFunction*>& hashes);

      static Parallel* make(const Spec& spec);
   private:
      Parallel() {}

      void add_data(const byte[], size_t);
      void final_result(byte[]);

      std::vector<std::unique_ptr<HashFunction>> hashes;
   };

}


namespace Botan {

/**
* Create a password hash using PBKDF2
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
* @param alg_id specifies which PRF to use with PBKDF2
*        0 is HMAC(SHA-1)
*        1 is HMAC(SHA-256)
*        2 is CMAC(Blowfish)
*        3 is HMAC(SHA-384)
*        4 is HMAC(SHA-512)
*        all other values are currently undefined
*/
std::string BOTAN_DLL generate_passhash9(const std::string& password,
                                         RandomNumberGenerator& rng,
                                         u16bit work_factor = 10,
                                         byte alg_id = 1);

/**
* Check a previously created password hash
* @param password the password to check against
* @param hash the stored hash to check against
*/
bool BOTAN_DLL check_passhash9(const std::string& password,
                               const std::string& hash);

}


namespace Botan {

/**
* Encrypt with PBES2 from PKCS #5 v2.0
* @param passphrase the passphrase to use for encryption
* @param msec how many milliseconds to run PBKDF2
* @param cipher specifies the block cipher to use to encrypt
* @param digest specifies the PRF to use with PBKDF2 (eg "HMAC(SHA-1)")
* @param rng a random number generator
*/
std::pair<AlgorithmIdentifier, std::vector<byte>>
BOTAN_DLL pbes2_encrypt(const secure_vector<byte>& key_bits,
                        const std::string& passphrase,
                        std::chrono::milliseconds msec,
                        const std::string& cipher,
                        const std::string& digest,
                        RandomNumberGenerator& rng);

/**
* Decrypt a PKCS #5 v2.0 encrypted stream
* @param key_bits the input
* @param passphrase the passphrase to use for decryption
* @param params the PBES2 parameters
*/
secure_vector<byte>
BOTAN_DLL pbes2_decrypt(const secure_vector<byte>& key_bits,
                        const std::string& passphrase,
                        const std::vector<byte>& params);

}


namespace Botan {

/**
* Base class for PBKDF (password based key derivation function)
* implementations. Converts a password into a key using a salt
* and iterated hashing to make brute force attacks harder.
*/
class BOTAN_DLL PBKDF
   {
   public:

      virtual ~PBKDF() {}

      typedef SCAN_Name Spec;

      /**
      * @return new instance of this same algorithm
      */
      virtual PBKDF* clone() const = 0;

      virtual std::string name() const = 0;

      /**
      * Derive a key from a passphrase for a number of iterations
      * specified by either iterations or if iterations == 0 then
      * running until seconds time has elapsed.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @return the number of iterations performed
      */
      virtual size_t pbkdf(byte out[], size_t out_len,
                           const std::string& passphrase,
                           const byte salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const = 0;

      void pbkdf_iterations(byte out[], size_t out_len,
                            const std::string& passphrase,
                            const byte salt[], size_t salt_len,
                            size_t iterations) const;

      void pbkdf_timed(byte out[], size_t out_len,
                         const std::string& passphrase,
                         const byte salt[], size_t salt_len,
                         std::chrono::milliseconds msec,
                         size_t& iterations) const;

      secure_vector<byte> pbkdf_iterations(size_t out_len,
                                           const std::string& passphrase,
                                           const byte salt[], size_t salt_len,
                                           size_t iterations) const;

      secure_vector<byte> pbkdf_timed(size_t out_len,
                                      const std::string& passphrase,
                                      const byte salt[], size_t salt_len,
                                      std::chrono::milliseconds msec,
                                      size_t& iterations) const;

      // Following kept for compat with 1.10:

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt, salt_len, iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param iterations the number of iterations to use (use 10K or more)
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, &salt[0], salt.size(), iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt, salt_len, msec, iterations);
         }

      /**
      * Derive a key from a passphrase using a certain amount of time
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, &salt[0], salt.size(), msec, iterations);
         }
   };

}


namespace Botan {

/**
* PKCS #5 v1 PBKDF, aka PBKDF1
* Can only generate a key up to the size of the hash output.
* Unless needed for backwards compatability, use PKCS5_PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF1 : public PBKDF
   {
   public:
      /**
      * Create a PKCS #5 instance using the specified hash function.
      * @param hash pointer to a hash function object to use
      */
      PKCS5_PBKDF1(HashFunction* hash) : m_hash(hash) {}

      std::string name() const
         {
         return "PBKDF1(" + m_hash->name() + ")";
         }

      PBKDF* clone() const
         {
         return new PKCS5_PBKDF1(m_hash->clone());
         }

      size_t pbkdf(byte output_buf[], size_t output_len,
                           const std::string& passphrase,
                           const byte salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const override;
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

BOTAN_DLL size_t pbkdf2(MessageAuthenticationCode& prf,
                        byte out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF2 : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(mac->clone());
         }

      size_t pbkdf(byte output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const byte salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : mac(mac_fn) {}

      static PKCS5_PBKDF2* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> mac;
   };

}


namespace Botan {

namespace PEM_Code {

/**
* Encode some binary data in PEM format
*/
BOTAN_DLL std::string encode(const byte data[],
                             size_t data_len,
                             const std::string& label,
                             size_t line_width = 64);

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const std::vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(&data[0], data.size(), label, line_width);
   }

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const secure_vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(&data[0], data.size(), label, line_width);
   }

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(DataSource& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(const std::string& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   DataSource& pem,
   const std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   const std::string& pem,
   const std::string& label);

/**
* Heuristic test for PEM data.
*/
BOTAN_DLL bool matches(DataSource& source,
                       const std::string& extra = "",
                       size_t search_range = 4096);

}

}


namespace Botan {

/**
* PKCS #10 Certificate Request.
*/
class BOTAN_DLL PKCS10_Request : public X509_Object
   {
   public:
      /**
      * Get the subject public key.
      * @return subject public key
      */
      Public_Key* subject_public_key() const;

      /**
      * Get the raw DER encoded public key.
      * @return raw DER encoded public key
      */
      std::vector<byte> raw_public_key() const;

      /**
      * Get the subject DN.
      * @return subject DN
      */
      X509_DN subject_dn() const;

      /**
      * Get the subject alternative name.
      * @return subject alternative name.
      */
      AlternativeName subject_alt_name() const;

      /**
      * Get the key constraints for the key associated with this
      * PKCS#10 object.
      * @return key constraints
      */
      Key_Constraints constraints() const;

      /**
      * Get the extendend key constraints (if any).
      * @return extended key constraints
      */
      std::vector<OID> ex_constraints() const;

      /**
      * Find out whether this is a CA request.
      * @result true if it is a CA request, false otherwise.
      */
      bool is_CA() const;

      /**
      * Return the constraint on the path length defined
      * in the BasicConstraints extension.
      * @return path limit
      */
      u32bit path_limit() const;

      /**
      * Get the challenge password for this request
      * @return challenge password for this request
      */
      std::string challenge_password() const;

      /**
      * Create a PKCS#10 Request from a data source.
      * @param source the data source providing the DER encoded request
      */
      PKCS10_Request(DataSource& source);

      /**
      * Create a PKCS#10 Request from a file.
      * @param filename the name of the file containing the DER or PEM
      * encoded request file
      */
      PKCS10_Request(const std::string& filename);

      /**
      * Create a PKCS#10 Request from binary data.
      * @param vec a std::vector containing the DER value
      */
      PKCS10_Request(const std::vector<byte>& vec);
   private:
      void force_decode();
      void handle_attribute(const Attribute&);

      Data_Store info;
   };

}


namespace Botan {

/**
* DJB's Poly1305
* Important note: each key can only be used once
*/
class BOTAN_DLL Poly1305 : public MessageAuthenticationCode
   {
   public:
      std::string name() const override { return "Poly1305"; }

      MessageAuthenticationCode* clone() const { return new Poly1305; }

      void clear();

      size_t output_length() const { return 16; }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(32);
         }

   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      secure_vector<u64bit> m_poly;
      secure_vector<byte> m_buf;
      size_t m_buf_pos = 0;
   };

}


namespace Botan {

/**
* PRF used in TLS 1.0/1.1
*/
class BOTAN_DLL TLS_PRF : public KDF
   {
   public:
      std::string name() const { return "TLS-PRF"; }

      KDF* clone() const { return new TLS_PRF; }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      TLS_PRF();
   private:
      std::unique_ptr<MessageAuthenticationCode> m_hmac_md5;
      std::unique_ptr<MessageAuthenticationCode> m_hmac_sha1;
   };

/**
* PRF used in TLS 1.2
*/
class BOTAN_DLL TLS_12_PRF : public KDF
   {
   public:
      std::string name() const { return "TLS-12-PRF(" + m_mac->name() + ")"; }

      KDF* clone() const { return new TLS_12_PRF(m_mac->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      TLS_12_PRF(MessageAuthenticationCode* mac) : m_mac(mac) {}

      static TLS_12_PRF* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}


namespace Botan {

/**
* PRF from ANSI X9.42
*/
class BOTAN_DLL X942_PRF : public KDF
   {
   public:
      std::string name() const { return "X942_PRF(" + m_key_wrap_oid + ")"; }

      KDF* clone() const { return new X942_PRF(m_key_wrap_oid); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      X942_PRF(const std::string& oid);
   private:
      std::string m_key_wrap_oid;
   };

}


namespace Botan {

/**
* PSSR (called EMSA4 in IEEE 1363 and in old versions of the library)
*/
class BOTAN_DLL PSSR : public EMSA
   {
   public:

      /**
      * @param hash the hash object to use
      */
      PSSR(HashFunction* hash);

      /**
      * @param hash the hash object to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR(HashFunction* hash, size_t salt_size);

      static PSSR* make(const Spec& spec);
   private:
      void update(const byte input[], size_t length);

      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits);

      size_t SALT_SIZE;
      std::unique_ptr<HashFunction> hash;
   };

}


namespace Botan {

/**
* RC2
*/
class BOTAN_DLL RC2 : public Block_Cipher_Fixed_Params<8, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      /**
      * Return the code of the effective key bits
      * @param bits key length
      * @return EKB code
      */
      static byte EKB_code(size_t bits);

      void clear();
      std::string name() const { return "RC2"; }
      BlockCipher* clone() const { return new RC2; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> K;
   };

}


namespace Botan {

/**
* RC4 stream cipher
*/
class BOTAN_DLL RC4 : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void clear();
      std::string name() const;

      StreamCipher* clone() const { return new RC4(SKIP); }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(1, 256);
         }

      static RC4* make(const Spec& spec);

      /**
      * @param skip skip this many initial bytes in the keystream
      */
      RC4(size_t skip = 0);

      ~RC4() { clear(); }
   private:
      void key_schedule(const byte[], size_t);
      void generate();

      const size_t SKIP;

      byte X, Y;
      secure_vector<byte> state;

      secure_vector<byte> buffer;
      size_t position;
   };

}


namespace Botan {

/**
* RC5
*/
class BOTAN_DLL RC5 : public Block_Cipher_Fixed_Params<8, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const;
      BlockCipher* clone() const { return new RC5(rounds); }

      /**
      * @param rounds the number of RC5 rounds to run. Must be between
      * 8 and 32 and a multiple of 4.
      */
      RC5(size_t rounds);
   private:
      void key_schedule(const byte[], size_t);

      size_t rounds;
      secure_vector<u32bit> S;
   };

}


namespace Botan {

/**
* RC6, Ron Rivest's AES candidate
*/
class BOTAN_DLL RC6 : public Block_Cipher_Fixed_Params<16, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "RC6"; }
      BlockCipher* clone() const { return new RC6; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> S;
   };

}


namespace Botan {

/**
* Encrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the plaintext key to encrypt
* @param kek the key encryption key
* @return key encrypted under kek
*/
secure_vector<byte> BOTAN_DLL rfc3394_keywrap(const secure_vector<byte>& key,
                                              const SymmetricKey& kek);

/**
* Decrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the encrypted key to decrypt
* @param kek the key encryption key
* @param af an algorithm factory
* @return key decrypted under kek
*/
secure_vector<byte> BOTAN_DLL rfc3394_keyunwrap(const secure_vector<byte>& key,
                                                const SymmetricKey& kek);

}


namespace Botan {

/**
* @param x the secret (EC)DSA key
* @param q the group order
* @param h the message hash already reduced mod q
* @param hash the hash function used to generate h
*/
BigInt BOTAN_DLL generate_rfc6979_nonce(const BigInt& x,
                                        const BigInt& q,
                                        const BigInt& h,
                                        const std::string& hash);

std::string hash_for_deterministic_signature(const std::string& emsa);

}


namespace Botan {

/**
* RIPEMD-128
*/
class BOTAN_DLL RIPEMD_128 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "RIPEMD-128"; }
      size_t output_length() const { return 16; }
      HashFunction* clone() const { return new RIPEMD_128; }

      void clear();

      RIPEMD_128() : MDx_HashFunction(64, false, true), M(16), digest(4)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u32bit> M, digest;
   };

}


namespace Botan {

/**
* RIPEMD-160
*/
class BOTAN_DLL RIPEMD_160 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "RIPEMD-160"; }
      size_t output_length() const { return 20; }
      HashFunction* clone() const { return new RIPEMD_160; }

      void clear();

      RIPEMD_160() : MDx_HashFunction(64, false, true), M(16), digest(5)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u32bit> M, digest;
   };

}



namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RSA"; }

      RSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {}

      /**
      * Create a RSA_PublicKey
      * @arg n the modulus
      * @arg e the exponent
      */
      RSA_PublicKey(const BigInt& n, const BigInt& e) :
         IF_Scheme_PublicKey(n, e)
         {}

   protected:
      RSA_PublicKey() {}
   };

/**
* RSA Private Key
*/
class BOTAN_DLL RSA_PrivateKey : public RSA_PublicKey,
                                 public IF_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      RSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits,
                     RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      /**
      * Construct a private key from the specified parameters.
      * @param rng a random number generator
      * @param p the first prime
      * @param q the second prime
      * @param e the exponent
      * @param d if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     const BigInt& p, const BigInt& q,
                     const BigInt& e, const BigInt& d = 0,
                     const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     size_t bits, size_t exp = 65537);
   };

}


namespace Botan {

/**
* Rabin-Williams Public Key
*/
class BOTAN_DLL RW_PublicKey : public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RW"; }

      RW_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {}

      RW_PublicKey(const BigInt& mod, const BigInt& exponent) :
         IF_Scheme_PublicKey(mod, exponent)
         {}

   protected:
      RW_PublicKey() {}
   };

/**
* Rabin-Williams Private Key
*/
class BOTAN_DLL RW_PrivateKey : public RW_PublicKey,
                                public IF_Scheme_PrivateKey
   {
   public:
      RW_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits,
                    RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      RW_PrivateKey(RandomNumberGenerator& rng,
                    const BigInt& p, const BigInt& q,
                    const BigInt& e, const BigInt& d = 0,
                    const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      RW_PrivateKey(RandomNumberGenerator& rng, size_t bits, size_t = 2);

      bool check_key(RandomNumberGenerator& rng, bool) const;
   };

}


namespace Botan {

/**
* SAFER-SK
*/
class BOTAN_DLL SAFER_SK : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const;
      BlockCipher* clone() const;

      /**
      * @param rounds the number of rounds to use - must be between 1
      * and 13
      */
      SAFER_SK(size_t rounds);
   private:
      void key_schedule(const byte[], size_t);

      size_t rounds;
      secure_vector<byte> EK;
   };

}


namespace Botan {

/**
* DJB's Salsa20 (and XSalsa20)
*/
class BOTAN_DLL Salsa20 : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len == 8 || iv_len == 24); }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear();
      std::string name() const;
      StreamCipher* clone() const { return new Salsa20; }
   private:
      void key_schedule(const byte key[], size_t key_len);

      secure_vector<u32bit> m_state;
      secure_vector<byte> m_buffer;
      size_t m_position;
   };

}


namespace Botan {

/**
* A queue that knows how to zeroize itself
*/
class BOTAN_DLL SecureQueue : public Fanout_Filter, public DataSource
   {
   public:
      std::string name() const { return "Queue"; }

      void write(const byte[], size_t);

      size_t read(byte[], size_t);
      size_t peek(byte[], size_t, size_t = 0) const;
      size_t get_bytes_read() const;

      bool end_of_data() const;

      bool empty() const;

      /**
      * @return number of bytes available in the queue
      */
      size_t size() const;

      bool attachable() { return false; }

      /**
      * SecureQueue assignment
      * @param other the queue to copy
      */
      SecureQueue& operator=(const SecureQueue& other);

      /**
      * SecureQueue default constructor (creates empty queue)
      */
      SecureQueue();

      /**
      * SecureQueue copy constructor
      * @param other the queue to copy
      */
      SecureQueue(const SecureQueue& other);

      ~SecureQueue() { destroy(); }
   private:
      size_t bytes_read;
      void destroy();
      class SecureQueueNode* head;
      class SecureQueueNode* tail;
   };

}


namespace Botan {

/**
* SEED, a Korean block cipher
*/
class BOTAN_DLL SEED : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "SEED"; }
      BlockCipher* clone() const { return new SEED; }
   private:
      void key_schedule(const byte[], size_t);

      class G_FUNC
         {
         public:
            u32bit operator()(u32bit) const;
         private:
            static const u32bit S0[256], S1[256], S2[256], S3[256];
         };

      secure_vector<u32bit> K;
   };

}


namespace Botan {

/**
* Serpent, an AES finalist
*/
class BOTAN_DLL Serpent : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Serpent"; }
      BlockCipher* clone() const { return new Serpent; }
   protected:
      /**
      * For use by subclasses using SIMD, asm, etc
      * @return const reference to the key schedule
      */
      const secure_vector<u32bit>& get_round_keys() const
         { return round_key; }

      /**
      * For use by subclasses that implement the key schedule
      * @param ks is the new key schedule value to set
      */
      void set_round_keys(const u32bit ks[132])
         {
         round_key.assign(&ks[0], &ks[132]);
         }

   private:
      void key_schedule(const byte key[], size_t length);
      secure_vector<u32bit> round_key;
   };

}


namespace Botan {

/**
* Serpent implementation using SIMD
*/
class BOTAN_DLL Serpent_SIMD : public Serpent
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new Serpent_SIMD; }
   };

}


namespace Botan {

/**
* NIST's SHA-160
*/
class BOTAN_DLL SHA_160 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-160"; }
      size_t output_length() const { return 20; }
      HashFunction* clone() const { return new SHA_160; }

      void clear();

      SHA_160() : MDx_HashFunction(64, true, true), digest(5), W(80)
         {
         clear();
         }
   protected:
      /**
      * Set a custom size for the W array. Normally 80, but some
      * subclasses need slightly more for best performance/internal
      * constraints
      * @param W_size how big to make W
      */
      SHA_160(size_t W_size) :
         MDx_HashFunction(64, true, true), digest(5), W(W_size)
         {
         clear();
         }

      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      /**
      * The digest value, exposed for use by subclasses (asm, SSE2)
      */
      secure_vector<u32bit> digest;

      /**
      * The message buffer, exposed for use by subclasses (asm, SSE2)
      */
      secure_vector<u32bit> W;
   };

}


namespace Botan {

/**
* SHA-160 using SSE2 for the message expansion
*/
class BOTAN_DLL SHA_160_SSE2 : public SHA_160
   {
   public:
      HashFunction* clone() const { return new SHA_160_SSE2; }
      SHA_160_SSE2() : SHA_160(0) {} // no W needed
   private:
      void compress_n(const byte[], size_t blocks);
   };

}


namespace Botan {

/**
* SHA-224
*/
class BOTAN_DLL SHA_224 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-224"; }
      size_t output_length() const { return 28; }
      HashFunction* clone() const { return new SHA_224; }

      void clear();

      SHA_224() : MDx_HashFunction(64, true, true), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u32bit> digest;
   };

/**
* SHA-256
*/
class BOTAN_DLL SHA_256 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-256"; }
      size_t output_length() const { return 32; }
      HashFunction* clone() const { return new SHA_256; }

      void clear();

      SHA_256() : MDx_HashFunction(64, true, true), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u32bit> digest;
   };

}


namespace Botan {

/**
* SHA-384
*/
class BOTAN_DLL SHA_384 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-384"; }
      size_t output_length() const { return 48; }
      HashFunction* clone() const { return new SHA_384; }

      void clear();

      SHA_384() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512
*/
class BOTAN_DLL SHA_512 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-512"; }
      size_t output_length() const { return 64; }
      HashFunction* clone() const { return new SHA_512; }

      void clear();

      SHA_512() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512/256
*/
class BOTAN_DLL SHA_512_256 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-512/256"; }
      size_t output_length() const { return 32; }
      HashFunction* clone() const { return new SHA_512_256; }

      void clear();

      SHA_512_256() : MDx_HashFunction(128, true, true, 16), m_digest(8) { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u64bit> m_digest;
   };

}


namespace Botan {

class BOTAN_DLL SipHash : public MessageAuthenticationCode
   {
   public:
      SipHash(size_t c = 2, size_t d = 4) : m_C(c), m_D(d) {}

      void clear();
      std::string name() const;

      MessageAuthenticationCode* clone() const;

      size_t output_length() const { return 8; }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(16);
         }
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      const size_t m_C, m_D;
      secure_vector<u64bit> m_V;
      u64bit m_mbuf = 0;
      size_t m_mbuf_pos = 0;
      byte m_words = 0;
   };

}


namespace Botan {

/**
* Base class for SIV encryption and decryption (@see RFC 5297)
*/
class BOTAN_DLL SIV_Mode : public AEAD_Mode
   {
   public:
      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void set_associated_data_n(size_t n, const byte ad[], size_t ad_len);

      void set_associated_data(const byte ad[], size_t ad_len) override
         {
         set_associated_data_n(0, ad, ad_len);
         }

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      void clear() override;

      size_t tag_size() const override { return 16; }

   protected:
      SIV_Mode(BlockCipher* cipher);

      StreamCipher& ctr() { return *m_ctr; }

      void set_ctr_iv(secure_vector<byte> V);

      secure_vector<byte>& msg_buf() { return m_msg_buf; }

      secure_vector<byte> S2V(const byte text[], size_t text_len);
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      const std::string m_name;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;
      secure_vector<byte> m_nonce, m_msg_buf;
      std::vector<secure_vector<byte>> m_ad_macs;
   };

/**
* SIV Encryption
*/
class BOTAN_DLL SIV_Encryption : public SIV_Mode
   {
   public:
      /**
      * @param cipher a block cipher
      */
      SIV_Encryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* SIV Decryption
*/
class BOTAN_DLL SIV_Decryption : public SIV_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      */
      SIV_Decryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}


namespace Botan {

/**
* Threefish-512
*/
class BOTAN_DLL Threefish_512 : public Block_Cipher_Fixed_Params<64, 64>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void set_tweak(const byte tweak[], size_t len);

      void clear() override;
      std::string name() const override { return "Threefish-512"; }
      BlockCipher* clone() const override { return new Threefish_512; }
   protected:
      const secure_vector<u64bit>& get_T() const { return m_T; }
      const secure_vector<u64bit>& get_K() const { return m_K; }
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      // Interface for Skein
      friend class Skein_512;

      virtual void skein_feedfwd(const secure_vector<u64bit>& M,
                                 const secure_vector<u64bit>& T);

      // Private data
      secure_vector<u64bit> m_T;
      secure_vector<u64bit> m_K;
   };

}


namespace Botan {

/**
* Skein-512, a SHA-3 candidate
*/
class BOTAN_DLL Skein_512 : public HashFunction
   {
   public:
      /**
      * @param output_bits the output size of Skein in bits
      * @param personalization is a string that will paramaterize the
      * hash output
      */
      Skein_512(size_t output_bits = 512,
                const std::string& personalization = "");

      size_t hash_block_size() const { return 64; }
      size_t output_length() const { return output_bits / 8; }

      static Skein_512* make(const Spec& spec);

      HashFunction* clone() const;
      std::string name() const;
      void clear();
   private:
      enum type_code {
         SKEIN_KEY = 0,
         SKEIN_CONFIG = 4,
         SKEIN_PERSONALIZATION = 8,
         SKEIN_PUBLIC_KEY = 12,
         SKEIN_KEY_IDENTIFIER = 16,
         SKEIN_NONCE = 20,
         SKEIN_MSG = 48,
         SKEIN_OUTPUT = 63
      };

      void add_data(const byte input[], size_t length);
      void final_result(byte out[]);

      void ubi_512(const byte msg[], size_t msg_len);

      void initial_block();
      void reset_tweak(type_code type, bool final);

      std::string personalization;
      size_t output_bits;

      std::unique_ptr<Threefish_512> m_threefish;
      secure_vector<u64bit> T;
      secure_vector<byte> buffer;
      size_t buf_pos;
   };

}


namespace Botan {

/**
* SRP6a Client side
* @param username the username we are attempting login for
* @param password the password we are attempting to use
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
* @param salt is the salt value sent by the server
* @param B is the server's public value
* @param rng is a random number generator
*
* @return (A,K) the client public key and the shared secret key
*/
std::pair<BigInt,SymmetricKey>
BOTAN_DLL srp6_client_agree(const std::string& username,
                            const std::string& password,
                            const std::string& group_id,
                            const std::string& hash_id,
                            const std::vector<byte>& salt,
                            const BigInt& B,
                            RandomNumberGenerator& rng);

/**
* Generate a new SRP-6 verifier
* @param identifier a username or other client identifier
* @param password the secret used to authenticate user
* @param salt a randomly chosen value, at least 128 bits long
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
*/
BigInt BOTAN_DLL generate_srp6_verifier(const std::string& identifier,
                                        const std::string& password,
                                        const std::vector<byte>& salt,
                                        const std::string& group_id,
                                        const std::string& hash_id);

/**
* Return the group id for this SRP param set, or else thrown an
* exception
* @param N the group modulus
* @param g the group generator
* @return group identifier
*/
std::string BOTAN_DLL srp6_group_identifier(const BigInt& N, const BigInt& g);

/**
* Represents a SRP-6a server session
*/
class BOTAN_DLL SRP6_Server_Session
   {
   public:
      /**
      * Server side step 1
      * @param v the verification value saved from client registration
      * @param group_id the SRP group id
      * @param hash_id the SRP hash in use
      * @param rng a random number generator
      * @return SRP-6 B value
      */
      BigInt step1(const BigInt& v,
                   const std::string& group_id,
                   const std::string& hash_id,
                   RandomNumberGenerator& rng);

      /**
      * Server side step 2
      * @param A the client's value
      * @return shared symmetric key
      */
      SymmetricKey step2(const BigInt& A);

   private:
      std::string m_hash_id;
      BigInt m_B, m_b, m_v, m_S, m_p;
      size_t m_p_bytes;
   };

}


namespace Botan {

/**
* A GnuTLS compatible SRP6 authenticator file
*/
class BOTAN_DLL SRP6_Authenticator_File
   {
   public:
      /**
      * @param filename will be opened and processed as a SRP
      * authenticator file
      */
      SRP6_Authenticator_File(const std::string& filename);

      bool lookup_user(const std::string& username,
                       BigInt& v,
                       std::vector<byte>& salt,
                       std::string& group_id) const;
   private:
      struct SRP6_Data
         {
         SRP6_Data() {}

         SRP6_Data(const BigInt& v,
                   const std::vector<byte>& salt,
                   const std::string& group_id) :
            v(v), salt(salt), group_id(group_id) {}

         BigInt v;
         std::vector<byte> salt;
         std::string group_id;
         };

      std::map<std::string, SRP6_Data> entries;
   };

}


namespace Botan {

class BOTAN_DLL Stream_Cipher_Mode : public Cipher_Mode
   {
   public:
      Stream_Cipher_Mode(StreamCipher* cipher) : m_cipher(cipher) {}

      void update(secure_vector<byte>& buf, size_t offset) override
         {
         if(offset < buf.size())
            m_cipher->cipher1(&buf[offset], buf.size() - offset);
         }

      void finish(secure_vector<byte>& buf, size_t offset) override
         { return update(buf, offset); }

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 64; /* arbitrary */ }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override { return m_cipher->clear(); }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override
         {
         m_cipher->set_iv(nonce, nonce_len);
         return secure_vector<byte>();
         }

      void key_schedule(const byte key[], size_t length)
         {
         m_cipher->set_key(key, length);
         }

      std::unique_ptr<StreamCipher> m_cipher;
   };

}


namespace Botan {

/**
* Return a shared reference to a global PRNG instance provided by the
* operating system. For instance might be instantiated by /dev/urandom
* or CryptGenRandom.
*/
BOTAN_DLL RandomNumberGenerator& system_rng();

}


namespace Botan {

/**
* TEA
*/
class BOTAN_DLL TEA : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "TEA"; }
      BlockCipher* clone() const { return new TEA; }
   private:
      void key_schedule(const byte[], size_t);
      secure_vector<u32bit> K;
   };

}


namespace Botan {

/**
* Threefish-512
*/
class BOTAN_DLL Threefish_512_AVX2 : public Threefish_512
   {
   private:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      /* TODO:
      void skein_feedfwd(const secure_vector<u64bit>& M,
                         const secure_vector<u64bit>& T) override;
      */

      BlockCipher* clone() const override { return new Threefish_512_AVX2; }
   };

}


namespace Botan {

/**
* Tiger
*/
class BOTAN_DLL Tiger : public MDx_HashFunction
   {
   public:
      std::string name() const;
      size_t output_length() const { return hash_len; }

      HashFunction* clone() const
         {
         return new Tiger(output_length(), passes);
         }

      void clear();

      /**
      * @param out_size specifies the output length; can be 16, 20, or 24
      * @param passes to make in the algorithm
      */
      Tiger(size_t out_size = 24, size_t passes = 3);
   private:
      void compress_n(const byte[], size_t block);
      void copy_out(byte[]);

      static void pass(u64bit& A, u64bit& B, u64bit& C,
                       const secure_vector<u64bit>& M,
                       byte mul);

      static const u64bit SBOX1[256];
      static const u64bit SBOX2[256];
      static const u64bit SBOX3[256];
      static const u64bit SBOX4[256];

      secure_vector<u64bit> X, digest;
      const size_t hash_len, passes;
   };

}


namespace Botan {

namespace TLS {

/**
* SSL/TLS Alert Message
*/
class BOTAN_DLL Alert
   {
   public:
      /**
      * Type codes for TLS alerts
      */
      enum Type {
         CLOSE_NOTIFY                    = 0,
         UNEXPECTED_MESSAGE              = 10,
         BAD_RECORD_MAC                  = 20,
         DECRYPTION_FAILED               = 21,
         RECORD_OVERFLOW                 = 22,
         DECOMPRESSION_FAILURE           = 30,
         HANDSHAKE_FAILURE               = 40,
         NO_CERTIFICATE                  = 41, // SSLv3 only
         BAD_CERTIFICATE                 = 42,
         UNSUPPORTED_CERTIFICATE         = 43,
         CERTIFICATE_REVOKED             = 44,
         CERTIFICATE_EXPIRED             = 45,
         CERTIFICATE_UNKNOWN             = 46,
         ILLEGAL_PARAMETER               = 47,
         UNKNOWN_CA                      = 48,
         ACCESS_DENIED                   = 49,
         DECODE_ERROR                    = 50,
         DECRYPT_ERROR                   = 51,
         EXPORT_RESTRICTION              = 60,
         PROTOCOL_VERSION                = 70,
         INSUFFICIENT_SECURITY           = 71,
         INTERNAL_ERROR                  = 80,
         INAPPROPRIATE_FALLBACK          = 86,
         USER_CANCELED                   = 90,
         NO_RENEGOTIATION                = 100,
         UNSUPPORTED_EXTENSION           = 110,
         CERTIFICATE_UNOBTAINABLE        = 111,
         UNRECOGNIZED_NAME               = 112,
         BAD_CERTIFICATE_STATUS_RESPONSE = 113,
         BAD_CERTIFICATE_HASH_VALUE      = 114,
         UNKNOWN_PSK_IDENTITY            = 115,

         // pseudo alert values
         NULL_ALERT                      = 256,
         HEARTBEAT_PAYLOAD               = 257
      };

      /**
      * @return true iff this alert is non-empty
      */
      bool is_valid() const { return (m_type_code != NULL_ALERT); }

      /**
      * @return if this alert is a fatal one or not
      */
      bool is_fatal() const { return m_fatal; }

      /**
      * @return type of alert
      */
      Type type() const { return m_type_code; }

      /**
      * @return type of alert
      */
      std::string type_string() const;

      /**
      * Serialize an alert
      */
      std::vector<byte> serialize() const;

      /**
      * Deserialize an Alert message
      * @param buf the serialized alert
      */
      Alert(const secure_vector<byte>& buf);

      /**
      * Create a new Alert
      * @param type_code the type of alert
      * @param fatal specifies if this is a fatal alert
      */
      Alert(Type type_code, bool fatal = false) :
         m_fatal(fatal), m_type_code(type_code) {}

      Alert() : m_fatal(false), m_type_code(NULL_ALERT) {}
   private:
      bool m_fatal;
      Type m_type_code;
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Protocol Version
*/
class BOTAN_DLL Protocol_Version
   {
   public:
      enum Version_Code {
         TLS_V10            = 0x0301,
         TLS_V11            = 0x0302,
         TLS_V12            = 0x0303,

         DTLS_V10           = 0xFEFF,
         DTLS_V12           = 0xFEFD
      };

      static Protocol_Version latest_tls_version()
         {
         return Protocol_Version(TLS_V12);
         }

      static Protocol_Version latest_dtls_version()
         {
         return Protocol_Version(DTLS_V12);
         }

      Protocol_Version() : m_version(0) {}

      /**
      * @param named_version a specific named version of the protocol
      */
      Protocol_Version(Version_Code named_version) :
         m_version(static_cast<u16bit>(named_version)) {}

      /**
      * @param major the major version
      * @param minor the minor version
      */
      Protocol_Version(byte major, byte minor) :
         m_version((static_cast<u16bit>(major) << 8) | minor) {}

      /**
      * @return true if this is a valid protocol version
      */
      bool valid() const { return (m_version != 0); }

      /**
      * @return true if this is a protocol version we know about
      */
      bool known_version() const;

      /**
      * @return major version of the protocol version
      */
      byte major_version() const { return get_byte(0, m_version); }

      /**
      * @return minor version of the protocol version
      */
      byte minor_version() const { return get_byte(1, m_version); }

      /**
      * @return human-readable description of this version
      */
      std::string to_string() const;

      /**
      * @return true iff this is a DTLS version
      */
      bool is_datagram_protocol() const;

      /**
      * @return true if this version supports negotiable signature algorithms
      */
      bool supports_negotiable_signature_algorithms() const;

      /**
      * @return true if this version uses explicit IVs for block ciphers
      */
      bool supports_explicit_cbc_ivs() const;

      /**
      * @return true if this version uses a ciphersuite specific PRF
      */
      bool supports_ciphersuite_specific_prf() const;

      bool supports_aead_modes() const;

      /**
      * @return if this version is equal to other
      */
      bool operator==(const Protocol_Version& other) const
         {
         return (m_version == other.m_version);
         }

      /**
      * @return if this version is not equal to other
      */
      bool operator!=(const Protocol_Version& other) const
         {
         return (m_version != other.m_version);
         }

      /**
      * @return if this version is later than other
      */
      bool operator>(const Protocol_Version& other) const;

      /**
      * @return if this version is later than or equal to other
      */
      bool operator>=(const Protocol_Version& other) const
         {
         return (*this == other || *this > other);
         }

   private:
      u16bit m_version;
   };

}

}


namespace Botan {

namespace TLS {

/**
* Ciphersuite Information
*/
class BOTAN_DLL Ciphersuite
   {
   public:
      /**
      * Convert an SSL/TLS ciphersuite to algorithm fields
      * @param suite the ciphersuite code number
      * @return ciphersuite object
      */
      static Ciphersuite by_id(u16bit suite);

      /**
      * Returns true iff this suite is a known SCSV
      */
      static bool is_scsv(u16bit suite);

      /**
      * Lookup a ciphersuite by name
      * @param name the name (eg TLS_RSA_WITH_RC4_128_SHA)
      * @return ciphersuite object
      */
      static Ciphersuite by_name(const std::string& name);

      /**
      * Generate a static list of all known ciphersuites and return it.
      *
      * @return list of all known ciphersuites
      */
      static const std::vector<Ciphersuite>& all_known_ciphersuites();

      /**
      * Formats the ciphersuite back to an RFC-style ciphersuite string
      * @return RFC ciphersuite string identifier
      */
      std::string to_string() const;

      /**
      * @return ciphersuite number
      */
      u16bit ciphersuite_code() const { return m_ciphersuite_code; }

      /**
      * @return true if this is a PSK ciphersuite
      */
      bool psk_ciphersuite() const;

      /**
      * @return true if this is an ECC ciphersuite
      */
      bool ecc_ciphersuite() const;

      /**
      * @return key exchange algorithm used by this ciphersuite
      */
      const std::string& kex_algo() const { return m_kex_algo; }

      /**
      * @return signature algorithm used by this ciphersuite
      */
      const std::string& sig_algo() const { return m_sig_algo; }

      /**
      * @return symmetric cipher algorithm used by this ciphersuite
      */
      const std::string& cipher_algo() const { return m_cipher_algo; }

      /**
      * @return message authentication algorithm used by this ciphersuite
      */
      const std::string& mac_algo() const { return m_mac_algo; }

      const std::string& prf_algo() const
         {
         return (m_prf_algo != "") ? m_prf_algo : m_mac_algo;
         }

      /**
      * @return cipher key length used by this ciphersuite
      */
      size_t cipher_keylen() const { return m_cipher_keylen; }

      size_t nonce_bytes_from_record() const { return m_nonce_bytes_from_record; }

      size_t nonce_bytes_from_handshake() const { return m_nonce_bytes_from_handshake; }

      size_t mac_keylen() const { return m_mac_keylen; }

      /**
      * @return true if this is a valid/known ciphersuite
      */
      bool valid() const;

      Ciphersuite() {}

   private:

      Ciphersuite(u16bit ciphersuite_code,
                  const char* sig_algo,
                  const char* kex_algo,
                  const char* cipher_algo,
                  size_t cipher_keylen,
                  size_t nonce_bytes_from_handshake,
                  size_t nonce_bytes_from_record,
                  const char* mac_algo,
                  size_t mac_keylen,
                  const char* prf_algo = "");

      u16bit m_ciphersuite_code = 0;

      std::string m_sig_algo;
      std::string m_kex_algo;
      std::string m_prf_algo;

      std::string m_cipher_algo;
      size_t m_cipher_keylen = 0;
      size_t m_nonce_bytes_from_handshake = 0;
      size_t m_nonce_bytes_from_record = 0;

      std::string m_mac_algo;
      size_t m_mac_keylen = 0;
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Policy Base Class
* Inherit and overload as desired to suit local policy concerns
*/
class BOTAN_DLL Policy
   {
   public:

      /**
      * Returns a list of ciphers we are willing to negotiate, in
      * order of preference.
      */
      virtual std::vector<std::string> allowed_ciphers() const;

      /**
      * Returns a list of hash algorithms we are willing to use for
      * signatures, in order of preference.
      */
      virtual std::vector<std::string> allowed_signature_hashes() const;

      /**
      * Returns a list of MAC algorithms we are willing to use.
      */
      virtual std::vector<std::string> allowed_macs() const;

      /**
      * Returns a list of key exchange algorithms we are willing to
      * use, in order of preference. Allowed values: DH, empty string
      * (representing RSA using server certificate key)
      */
      virtual std::vector<std::string> allowed_key_exchange_methods() const;

      /**
      * Returns a list of signature algorithms we are willing to
      * use, in order of preference. Allowed values RSA and DSA.
      */
      virtual std::vector<std::string> allowed_signature_methods() const;

      /**
      * Return list of ECC curves we are willing to use in order of preference
      */
      virtual std::vector<std::string> allowed_ecc_curves() const;

      /**
      * Returns a list of compression algorithms we are willing to use,
      * in order of preference. Allowed values any value of
      * Compression_Method.
      *
      * @note Compression is not currently supported
      */
      virtual std::vector<byte> compression() const;

      /**
      * Choose an elliptic curve to use
      */
      virtual std::string choose_curve(const std::vector<std::string>& curve_names) const;

      /**
      * Attempt to negotiate the use of the heartbeat extension
      */
      virtual bool negotiate_heartbeat_support() const;

      /**
      * Allow renegotiation even if the counterparty doesn't
      * support the secure renegotiation extension.
      *
      * @warning Changing this to true exposes you to injected
      * plaintext attacks. Read RFC 5746 for background.
      */
      virtual bool allow_insecure_renegotiation() const;

      /**
      * The protocol dictates that the first 32 bits of the random
      * field are the current time in seconds. However this allows
      * client fingerprinting attacks. Set to false to disable, in
      * which case random bytes will be used instead.
      */
      virtual bool include_time_in_hello_random() const;

      /**
      * Allow servers to initiate a new handshake
      */
      virtual bool allow_server_initiated_renegotiation() const;

      virtual std::string dh_group() const;

      /**
      * Return the minimum DH group size we're willing to use
      */
      virtual size_t minimum_dh_group_size() const;

      /**
      * If this function returns false, unknown SRP/PSK identifiers
      * will be rejected with an unknown_psk_identifier alert as soon
      * as the non-existence is identified. Otherwise, a false
      * identifier value will be used and the protocol allowed to
      * proceed, causing the handshake to eventually fail without
      * revealing that the username does not exist on this system.
      */
      virtual bool hide_unknown_users() const;

      /**
      * Return the allowed lifetime of a session ticket. If 0, session
      * tickets do not expire until the session ticket key rolls over.
      * Expired session tickets cannot be used to resume a session.
      */
      virtual u32bit session_ticket_lifetime() const;

      /**
      * If this returns a non-empty vector, and DTLS is negotiated,
      * then we will also attempt to negotiate the SRTP extension from
      * RFC 5764 using the returned values as the profile ids.
      */
      virtual std::vector<u16bit> srtp_profiles() const;

      /**
      * @return true if and only if we are willing to accept this version
      * Default accepts TLS v1.0 and later or DTLS v1.2 or later.
      */
      virtual bool acceptable_protocol_version(Protocol_Version version) const;

      /**
      * Returns the more recent protocol version we are willing to
      * use, for either TLS or DTLS depending on datagram param.
      * Shouldn't ever need to override this unless you want to allow
      * a user to disable use of TLS v1.2 (which is *not recommended*)
      */
      virtual Protocol_Version latest_supported_version(bool datagram) const;

      /**
      * When offering this version, should we send a fallback SCSV?
      * Default returns true iff version is not the latest version the
      * policy allows, exists to allow override in case of interop problems.
      */
      virtual bool send_fallback_scsv(Protocol_Version version) const;

      /**
      * Allows policy to reject any ciphersuites which are undesirable
      * for whatever reason without having to reimplement ciphersuite_list
      */
      virtual bool acceptable_ciphersuite(const Ciphersuite& suite) const;

      /**
      * @return true if servers should choose the ciphersuite matching
      *         their highest preference, rather than the clients.
      *         Has no effect on client side.
      */
      virtual bool server_uses_own_ciphersuite_preferences() const;

      /**
      * Return allowed ciphersuites, in order of preference
      */
      virtual std::vector<u16bit> ciphersuite_list(Protocol_Version version,
                                                   bool have_srp) const;

      virtual void print(std::ostream& o) const;

      virtual ~Policy() {}
   };

/**
* NSA Suite B 128-bit security level (see @rfc 6460)
*/
class BOTAN_DLL NSA_Suite_B_128 : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override
         { return std::vector<std::string>({"AES-128/GCM"}); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return std::vector<std::string>({"SHA-256"}); }

      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return std::vector<std::string>({"ECDH"}); }

      std::vector<std::string> allowed_signature_methods() const override
         { return std::vector<std::string>({"ECDSA"}); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return std::vector<std::string>({"secp256r1"}); }

      bool acceptable_protocol_version(Protocol_Version version) const override
         { return version == Protocol_Version::TLS_V12; }
   };

/**
* Policy for DTLS. We require DTLS v1.2 and an AEAD mode
*/
class BOTAN_DLL Datagram_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }

      bool acceptable_protocol_version(Protocol_Version version) const override
         { return version == Protocol_Version::DTLS_V12; }
   };

/*
* This policy requires a secure version of TLS and disables all insecure
* algorithms. It is compatible with other botan TLSes (including those using the
* default policy) and with many other recent implementations. It is a great idea
* to use if you control both sides of the protocol and don't have to worry
* about ancient and/or bizarre TLS implementations.
*/
class BOTAN_DLL Strict_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override;

      std::vector<std::string> allowed_signature_hashes() const override;

      std::vector<std::string> allowed_macs() const override;

      std::vector<std::string> allowed_key_exchange_methods() const override;

      bool acceptable_protocol_version(Protocol_Version version) const override;
   };

class BOTAN_DLL Text_Policy : public Policy
   {
   public:

      std::vector<std::string> allowed_ciphers() const override
         { return get_list("ciphers", Policy::allowed_ciphers()); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return get_list("signature_hashes", Policy::allowed_signature_hashes()); }

      std::vector<std::string> allowed_macs() const override
         { return get_list("macs", Policy::allowed_macs()); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return get_list("key_exchange_methods", Policy::allowed_key_exchange_methods()); }

      std::vector<std::string> allowed_signature_methods() const override
         { return get_list("signature_methods", Policy::allowed_signature_methods()); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return get_list("ecc_curves", Policy::allowed_ecc_curves()); }

      bool negotiate_heartbeat_support() const override
         { return get_bool("negotiate_heartbeat_support", Policy::negotiate_heartbeat_support()); }

      bool allow_insecure_renegotiation() const override
         { return get_bool("allow_insecure_renegotiation", Policy::allow_insecure_renegotiation()); }

      bool include_time_in_hello_random() const override
         { return get_bool("include_time_in_hello_random", Policy::include_time_in_hello_random()); }

      bool allow_server_initiated_renegotiation() const override
         { return get_bool("allow_server_initiated_renegotiation", Policy::allow_server_initiated_renegotiation()); }

      bool server_uses_own_ciphersuite_preferences() const override
         { return get_bool("server_uses_own_ciphersuite_preferences", Policy::server_uses_own_ciphersuite_preferences()); }

      std::string dh_group() const override
         { return get_str("dh_group", Policy::dh_group()); }

      size_t minimum_dh_group_size() const override
         { return get_len("minimum_dh_group_size", Policy::minimum_dh_group_size()); }

      bool hide_unknown_users() const override
         { return get_bool("hide_unknown_users", Policy::hide_unknown_users()); }

      u32bit session_ticket_lifetime() const override
         { return get_len("session_ticket_lifetime", Policy::session_ticket_lifetime()); }

      std::vector<u16bit> srtp_profiles() const override
         {
         std::vector<u16bit> r;
         for(auto&& p : get_list("srtp_profiles", std::vector<std::string>()))
            {
            r.push_back(to_u32bit(p));
            }
         return r;
         }

      Text_Policy(std::istream& in)
         {
         m_kv = read_cfg(in);
         }

   private:

      std::vector<std::string> get_list(const std::string& key,
                                        const std::vector<std::string>& def) const
         {
         const std::string v = get_str(key);

         if(v == "")
            return def;

         return split_on(v, ' ');
         }

      size_t get_len(const std::string& key, size_t def) const
         {
         const std::string v = get_str(key);

         if(v == "")
            return def;

         return to_u32bit(v);
         }

      bool get_bool(const std::string& key, bool def) const
         {
         const std::string v = get_str(key);

         if(v == "")
            return def;

         if(v == "true" || v == "True")
            return true;
         else if(v == "false" || v == "False")
            return false;
         else
            throw std::runtime_error("Invalid boolean '" + v + "'");
         }

      std::string get_str(const std::string& key, const std::string& def = "") const
         {
         auto i = m_kv.find(key);
         if(i == m_kv.end())
            return def;

         return i->second;
         }

      std::map<std::string, std::string> m_kv;
   };

}

}

namespace Botan {

namespace TLS {

/**
* Protocol Constants for SSL/TLS
*/
enum Size_Limits {
   TLS_HEADER_SIZE    = 5,
   DTLS_HEADER_SIZE   = TLS_HEADER_SIZE + 8,

   MAX_PLAINTEXT_SIZE = 16*1024,
   MAX_COMPRESSED_SIZE = MAX_PLAINTEXT_SIZE + 1024,
   MAX_CIPHERTEXT_SIZE = MAX_COMPRESSED_SIZE + 1024,
};

enum Connection_Side { CLIENT = 1, SERVER = 2 };

enum Record_Type {
   CHANGE_CIPHER_SPEC = 20,
   ALERT              = 21,
   HANDSHAKE          = 22,
   APPLICATION_DATA   = 23,
   HEARTBEAT          = 24,

   NO_RECORD          = 256
};

enum Handshake_Type {
   HELLO_REQUEST        = 0,
   CLIENT_HELLO         = 1,
   SERVER_HELLO         = 2,
   HELLO_VERIFY_REQUEST = 3,
   NEW_SESSION_TICKET   = 4, // RFC 5077
   CERTIFICATE          = 11,
   SERVER_KEX           = 12,
   CERTIFICATE_REQUEST  = 13,
   SERVER_HELLO_DONE    = 14,
   CERTIFICATE_VERIFY   = 15,
   CLIENT_KEX           = 16,
   FINISHED             = 20,

   CERTIFICATE_URL      = 21,
   CERTIFICATE_STATUS   = 22,

   NEXT_PROTOCOL        = 67,

   HANDSHAKE_CCS        = 254, // Not a wire value
   HANDSHAKE_NONE       = 255  // Null value
};

enum Compression_Method {
   NO_COMPRESSION       = 0x00,
   DEFLATE_COMPRESSION  = 0x01
};

}

}


namespace Botan {

namespace TLS {

/**
* Represents information known about a TLS server.
*/
class BOTAN_DLL Server_Information
   {
   public:
      /**
      * An empty server info - nothing known
      */
      Server_Information() : m_hostname(""), m_service(""), m_port(0) {}

      /**
      * @param hostname the host's DNS name, if known
      * @param port specifies the protocol port of the server (eg for
      *        TCP/UDP). Zero represents unknown.
      */
      Server_Information(const std::string& hostname,
                        u16bit port = 0) :
         m_hostname(hostname), m_service(""), m_port(port) {}

      /**
      * @param hostname the host's DNS name, if known
      * @param service is a text string of the service type
      *        (eg "https", "tor", or "git")
      * @param port specifies the protocol port of the server (eg for
      *        TCP/UDP). Zero represents unknown.
      */
      Server_Information(const std::string& hostname,
                        const std::string& service,
                        u16bit port = 0) :
         m_hostname(hostname), m_service(service), m_port(port) {}

      std::string hostname() const { return m_hostname; }

      std::string service() const { return m_service; }

      u16bit port() const { return m_port; }

      bool empty() const { return m_hostname.empty(); }

   private:
      std::string m_hostname, m_service;
      u16bit m_port;
   };

inline bool operator==(const Server_Information& a, const Server_Information& b)
   {
   return (a.hostname() == b.hostname()) &&
          (a.service() == b.service()) &&
          (a.port() == b.port());

   }

inline bool operator!=(const Server_Information& a, const Server_Information& b)
   {
   return !(a == b);
   }

inline bool operator<(const Server_Information& a, const Server_Information& b)
   {
   if(a.hostname() != b.hostname())
      return (a.hostname() < b.hostname());
   if(a.service() != b.service())
      return (a.service() < b.service());
   if(a.port() != b.port())
      return (a.port() < b.port());
   return false; // equal
   }

}

}


namespace Botan {

namespace TLS {

/**
* Class representing a TLS session state
*/
class BOTAN_DLL Session
   {
   public:

      /**
      * Uninitialized session
      */
      Session() :
         m_start_time(std::chrono::system_clock::time_point::min()),
         m_version(),
         m_ciphersuite(0),
         m_compression_method(0),
         m_connection_side(static_cast<Connection_Side>(0)),
         m_fragment_size(0)
            {}

      /**
      * New session (sets session start time)
      */
      Session(const std::vector<byte>& session_id,
              const secure_vector<byte>& master_secret,
              Protocol_Version version,
              u16bit ciphersuite,
              byte compression_method,
              Connection_Side side,
              size_t fragment_size,
              const std::vector<X509_Certificate>& peer_certs,
              const std::vector<byte>& session_ticket,
              const Server_Information& server_info,
              const std::string& srp_identifier,
              u16bit srtp_profile);

      /**
      * Load a session from DER representation (created by DER_encode)
      */
      Session(const byte ber[], size_t ber_len);

      /**
      * Load a session from PEM representation (created by PEM_encode)
      */
      Session(const std::string& pem);

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      secure_vector<byte> DER_encode() const;

      /**
      * Encrypt a session (useful for serialization or session tickets)
      */
      std::vector<byte> encrypt(const SymmetricKey& key,
                                RandomNumberGenerator& rng) const;


      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param ctext_size the size of ctext in bytes
      * @param key the same key used by the encrypting side
      */
      static Session decrypt(const byte ctext[],
                             size_t ctext_size,
                             const SymmetricKey& key);

      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param key the same key used by the encrypting side
      */
      static inline Session decrypt(const std::vector<byte>& ctext,
                                    const SymmetricKey& key)
         {
         return Session::decrypt(&ctext[0], ctext.size(), key);
         }

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      std::string PEM_encode() const;

      /**
      * Get the version of the saved session
      */
      Protocol_Version version() const { return m_version; }

      /**
      * Get the ciphersuite code of the saved session
      */
      u16bit ciphersuite_code() const { return m_ciphersuite; }

      /**
      * Get the ciphersuite info of the saved session
      */
      Ciphersuite ciphersuite() const { return Ciphersuite::by_id(m_ciphersuite); }

      /**
      * Get the compression method used in the saved session
      */
      byte compression_method() const { return m_compression_method; }

      /**
      * Get which side of the connection the resumed session we are/were
      * acting as.
      */
      Connection_Side side() const { return m_connection_side; }

      /**
      * Get the SRP identity (if sent by the client in the initial handshake)
      */
      const std::string& srp_identifier() const { return m_srp_identifier; }

      /**
      * Get the saved master secret
      */
      const secure_vector<byte>& master_secret() const { return m_master_secret; }

      /**
      * Get the session identifier
      */
      const std::vector<byte>& session_id() const { return m_identifier; }

      /**
      * Get the negotiated maximum fragment size (or 0 if default)
      */
      size_t fragment_size() const { return m_fragment_size; }

      /**
      * Get the negotiated DTLS-SRTP algorithm (RFC 5764)
      */
      u16bit dtls_srtp_profile() const { return m_srtp_profile; }

      /**
      * Return the certificate chain of the peer (possibly empty)
      */
      const std::vector<X509_Certificate>& peer_certs() const { return m_peer_certs; }

      /**
      * Get the wall clock time this session began
      */
      std::chrono::system_clock::time_point start_time() const { return m_start_time; }

      /**
      * Return how long this session has existed (in seconds)
      */
      std::chrono::seconds session_age() const;

      /**
      * Return the session ticket the server gave us
      */
      const std::vector<byte>& session_ticket() const { return m_session_ticket; }

      const Server_Information& server_info() const { return m_server_info; }

   private:
      enum { TLS_SESSION_PARAM_STRUCT_VERSION = 20150104 };

      std::chrono::system_clock::time_point m_start_time;

      std::vector<byte> m_identifier;
      std::vector<byte> m_session_ticket; // only used by client side
      secure_vector<byte> m_master_secret;

      Protocol_Version m_version;
      u16bit m_ciphersuite;
      byte m_compression_method;
      Connection_Side m_connection_side;
      u16bit m_srtp_profile;

      size_t m_fragment_size;

      std::vector<X509_Certificate> m_peer_certs;
      Server_Information m_server_info; // optional
      std::string m_srp_identifier; // optional
   };

}

}


namespace Botan {

namespace TLS {

/**
* Session_Manager is an interface to systems which can save
* session parameters for supporting session resumption.
*
* Saving sessions is done on a best-effort basis; an implementation is
* allowed to drop sessions due to space constraints.
*
* Implementations should strive to be thread safe
*/
class BOTAN_DLL Session_Manager
   {
   public:
      /**
      * Try to load a saved session (using session ID)
      * @param session_id the session identifier we are trying to resume
      * @param session will be set to the saved session data (if found),
               or not modified if not found
      * @return true if session was modified
      */
      virtual bool load_from_session_id(const std::vector<byte>& session_id,
                                        Session& session) = 0;

      /**
      * Try to load a saved session (using info about server)
      * @param info the information about the server
      * @param session will be set to the saved session data (if found),
               or not modified if not found
      * @return true if session was modified
      */
      virtual bool load_from_server_info(const Server_Information& info,
                                         Session& session) = 0;

      /**
      * Remove this session id from the cache, if it exists
      */
      virtual void remove_entry(const std::vector<byte>& session_id) = 0;

      /**
      * Save a session on a best effort basis; the manager may not in
      * fact be able to save the session for whatever reason; this is
      * not an error. Caller cannot assume that calling save followed
      * immediately by load_from_* will result in a successful lookup.
      *
      * @param session to save
      */
      virtual void save(const Session& session) = 0;

      /**
      * Return the allowed lifetime of a session; beyond this time,
      * sessions are not resumed. Returns 0 if unknown/no explicit
      * expiration policy.
      */
      virtual std::chrono::seconds session_lifetime() const = 0;

      virtual ~Session_Manager() {}
   };

/**
* An implementation of Session_Manager that does not save sessions at
* all, preventing session resumption.
*/
class BOTAN_DLL Session_Manager_Noop : public Session_Manager
   {
   public:
      bool load_from_session_id(const std::vector<byte>&, Session&) override
         { return false; }

      bool load_from_server_info(const Server_Information&, Session&) override
         { return false; }

      void remove_entry(const std::vector<byte>&) override {}

      void save(const Session&) override {}

      std::chrono::seconds session_lifetime() const override
         { return std::chrono::seconds(0); }
   };

/**
* An implementation of Session_Manager that saves values in memory.
*/
class BOTAN_DLL Session_Manager_In_Memory : public Session_Manager
   {
   public:
      /**
      * @param max_sessions a hint on the maximum number of sessions
      *        to keep in memory at any one time. (If zero, don't cap)
      * @param session_lifetime sessions are expired after this many
      *        seconds have elapsed from initial handshake.
      */
      Session_Manager_In_Memory(RandomNumberGenerator& rng,
                                size_t max_sessions = 1000,
                                std::chrono::seconds session_lifetime =
                                   std::chrono::seconds(7200));

      bool load_from_session_id(const std::vector<byte>& session_id,
                                Session& session) override;

      bool load_from_server_info(const Server_Information& info,
                                 Session& session) override;

      void remove_entry(const std::vector<byte>& session_id) override;

      void save(const Session& session_data) override;

      std::chrono::seconds session_lifetime() const override
         { return m_session_lifetime; }

   private:
      bool load_from_session_str(const std::string& session_str,
                                 Session& session);

      std::mutex m_mutex;

      size_t m_max_sessions;

      std::chrono::seconds m_session_lifetime;

      RandomNumberGenerator& m_rng;
      SymmetricKey m_session_key;

      std::map<std::string, std::vector<byte>> m_sessions; // hex(session_id) -> session
      std::map<Server_Information, std::string> m_info_sessions;
   };

}

}


namespace Botan {

namespace TLS {

class Connection_Cipher_State;
class Connection_Sequence_Numbers;
class Handshake_State;

/**
* Generic interface for TLS endpoint
*/
class BOTAN_DLL Channel
   {
   public:
      typedef std::function<void (const byte[], size_t)> output_fn;
      typedef std::function<void (const byte[], size_t)> data_cb;
      typedef std::function<void (Alert, const byte[], size_t)> alert_cb;
      typedef std::function<bool (const Session&)> handshake_cb;

      Channel(output_fn out,
              data_cb app_data_cb,
              alert_cb alert_cb,
              handshake_cb hs_cb,
              Session_Manager& session_manager,
              RandomNumberGenerator& rng,
              bool is_datagram,
              size_t reserved_io_buffer_size);

      Channel(const Channel&) = delete;

      Channel& operator=(const Channel&) = delete;

      virtual ~Channel();

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const byte buf[], size_t buf_size);

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const std::vector<byte>& buf);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const byte buf[], size_t buf_size);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const std::string& val);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      template<typename Alloc>
         void send(const std::vector<unsigned char, Alloc>& val)
         {
         send(&val[0], val.size());
         }

      /**
      * Send a TLS alert message. If the alert is fatal, the internal
      * state (keys, etc) will be reset.
      * @param alert the Alert to send
      */
      void send_alert(const Alert& alert);

      /**
      * Send a warning alert
      */
      void send_warning_alert(Alert::Type type) { send_alert(Alert(type, false)); }

      /**
      * Send a fatal alert
      */
      void send_fatal_alert(Alert::Type type) { send_alert(Alert(type, true)); }

      /**
      * Send a close notification alert
      */
      void close() { send_warning_alert(Alert::CLOSE_NOTIFY); }

      /**
      * @return true iff the connection is active for sending application data
      */
      bool is_active() const;

      /**
      * @return true iff the connection has been definitely closed
      */
      bool is_closed() const;


      /**
      * @return certificate chain of the peer (may be empty)
      */
      std::vector<X509_Certificate> peer_cert_chain() const;

      /**
      * Key material export (RFC 5705)
      * @param label a disambiguating label string
      * @param context a per-association context value
      * @param length the length of the desired key in bytes
      * @return key of length bytes
      */
      SymmetricKey key_material_export(const std::string& label,
                                       const std::string& context,
                                       size_t length) const;

      /**
      * Attempt to renegotiate the session
      * @param force_full_renegotiation if true, require a full renegotiation,
      * otherwise allow session resumption
      */
      void renegotiate(bool force_full_renegotiation = false);

      /**
      * @return true iff the counterparty supports the secure
      * renegotiation extensions.
      */
      bool secure_renegotiation_supported() const;

      /**
      * Perform a handshake timeout check. This does nothing unless
      * this is a DTLS channel with a pending handshake state, in
      * which case we check for timeout and potentially retransmit
      * handshake packets.
      */
      bool timeout_check();

      /**
      * @return true iff the peer supports heartbeat messages
      */
      bool peer_supports_heartbeats() const;

      /**
      * @return true iff we are allowed to send heartbeat messages
      */
      bool heartbeat_sending_allowed() const;

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      * @param payload will be echoed back
      * @param payload_size size of payload in bytes
      * @param pad_bytes include 16 + pad_bytes extra bytes in the message (not echoed)
      */
      void heartbeat(const byte payload[], size_t payload_size, size_t pad_bytes = 0);

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      */
      void heartbeat() { heartbeat(nullptr, 0); }
   protected:

      virtual void process_handshake_msg(const Handshake_State* active_state,
                                         Handshake_State& pending_state,
                                         Handshake_Type type,
                                         const std::vector<byte>& contents) = 0;

      virtual void initiate_handshake(Handshake_State& state,
                                      bool force_full_renegotiation) = 0;

      virtual std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const = 0;

      virtual Handshake_State* new_handshake_state(class Handshake_IO* io) = 0;

      Handshake_State& create_handshake_state(Protocol_Version version);

      void activate_session();

      void change_cipher_spec_reader(Connection_Side side);

      void change_cipher_spec_writer(Connection_Side side);

      /* secure renegotiation handling */

      void secure_renegotiation_check(const class Client_Hello* client_hello);
      void secure_renegotiation_check(const class Server_Hello* server_hello);

      std::vector<byte> secure_renegotiation_data_for_client_hello() const;
      std::vector<byte> secure_renegotiation_data_for_server_hello() const;

      RandomNumberGenerator& rng() { return m_rng; }

      Session_Manager& session_manager() { return m_session_manager; }

      bool save_session(const Session& session) const { return m_handshake_cb(session); }

   private:
      size_t maximum_fragment_size() const;

      void send_record(byte record_type, const std::vector<byte>& record);

      void send_record_under_epoch(u16bit epoch, byte record_type,
                                   const std::vector<byte>& record);

      void send_record_array(u16bit epoch, byte record_type,
                             const byte input[], size_t length);

      void write_record(Connection_Cipher_State* cipher_state,
                        u16bit epoch, byte type, const byte input[], size_t length);

      Connection_Sequence_Numbers& sequence_numbers() const;

      std::shared_ptr<Connection_Cipher_State> read_cipher_state_epoch(u16bit epoch) const;

      std::shared_ptr<Connection_Cipher_State> write_cipher_state_epoch(u16bit epoch) const;

      void reset_state();

      const Handshake_State* active_state() const { return m_active_state.get(); }

      const Handshake_State* pending_state() const { return m_pending_state.get(); }

      bool m_is_datagram;

      /* callbacks */
      handshake_cb m_handshake_cb;
      data_cb m_data_cb;
      alert_cb m_alert_cb;
      output_fn m_output_fn;

      /* external state */
      RandomNumberGenerator& m_rng;
      Session_Manager& m_session_manager;

      /* sequence number state */
      std::unique_ptr<Connection_Sequence_Numbers> m_sequence_numbers;

      /* pending and active connection states */
      std::unique_ptr<Handshake_State> m_active_state;
      std::unique_ptr<Handshake_State> m_pending_state;

      /* cipher states for each epoch */
      std::map<u16bit, std::shared_ptr<Connection_Cipher_State>> m_write_cipher_states;
      std::map<u16bit, std::shared_ptr<Connection_Cipher_State>> m_read_cipher_states;

      /* I/O buffers */
      secure_vector<byte> m_writebuf;
      secure_vector<byte> m_readbuf;
   };

}

}


namespace Botan {

namespace TLS {

/**
* SSL/TLS Client
*/
class BOTAN_DLL Client : public Channel
   {
   public:
      /**
      * Set up a new TLS client session
      *
      * @param output_fn is called with data for the outbound socket
      *
      * @param app_data_cb is called when new application data is received
      *
      * @param alert_cb is called when a TLS alert is received
      *
      * @param handshake_cb is called when a handshake is completed
      *
      * @param session_manager manages session state
      *
      * @param creds manages application/user credentials
      *
      * @param policy specifies other connection policy information
      *
      * @param rng a random number generator
      *
      * @param server_info is identifying information about the TLS server
      *
      * @param offer_version specifies which version we will offer
      *        to the TLS server.
      *
      * @param next_protocol allows the client to specify what the next
      *        protocol will be. For more information read
      *        http://technotes.googlecode.com/git/nextprotoneg.html.
      *
      *        If the function is not empty, NPN will be negotiated
      *        and if the server supports NPN the function will be
      *        called with the list of protocols the server advertised;
      *        the client should return the protocol it would like to use.
      *
      * @param reserved_io_buffer_size This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */

      typedef std::function<std::string (std::vector<std::string>)> next_protocol_fn;

      Client(output_fn out,
             data_cb app_data_cb,
             alert_cb alert_cb,
             handshake_cb hs_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const Server_Information& server_info = Server_Information(),
             const Protocol_Version offer_version = Protocol_Version::latest_tls_version(),
             next_protocol_fn next_protocol =  next_protocol_fn(),
             size_t reserved_io_buffer_size = 16*1024
         );
   private:
      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void send_client_hello(Handshake_State& state,
                             bool force_full_renegotiation,
                             Protocol_Version version,
                             const std::string& srp_identifier = "",
                             next_protocol_fn next_protocol = next_protocol_fn());

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<byte>& contents) override;

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      const Policy& m_policy;
      Credentials_Manager& m_creds;
      const Server_Information m_info;
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Server
*/
class BOTAN_DLL Server : public Channel
   {
   public:
      /**
      * Server initialization
      */
      Server(output_fn output,
             data_cb data_cb,
             alert_cb alert_cb,
             handshake_cb handshake_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const std::vector<std::string>& protocols = std::vector<std::string>(),
             bool is_datagram = false,
             size_t reserved_io_buffer_size = 16*1024
         );

      /**
      * Return the protocol notification set by the client (using the
      * NPN extension) for this connection, if any. This value is not
      * tied to the session and a later renegotiation of the same
      * session can choose a new protocol.
      */
      std::string next_protocol() const { return m_next_protocol; }

   private:
      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<byte>& contents) override;

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      const Policy& m_policy;
      Credentials_Manager& m_creds;

      std::vector<std::string> m_possible_protocols;
      std::string m_next_protocol;
   };

}

}


namespace Botan {

//template<typename T> using secure_deque = std::vector<T, secure_allocator<T>>;

namespace TLS {

/**
* Blocking TLS Client
* Can be used directly, or subclass to get handshake and alert notifications
*/
class BOTAN_DLL Blocking_Client
   {
   public:
      /*
      * These functions are expected to block until completing entirely, or
      * fail by throwing an exception.
      */
      typedef std::function<size_t (byte[], size_t)> read_fn;
      typedef std::function<void (const byte[], size_t)> write_fn;

      typedef Client::next_protocol_fn next_protocol_fn;

       Blocking_Client(read_fn reader,
                       write_fn writer,
                       Session_Manager& session_manager,
                       Credentials_Manager& creds,
                       const Policy& policy,
                       RandomNumberGenerator& rng,
                       const Server_Information& server_info = Server_Information(),
                       const Protocol_Version offer_version = Protocol_Version::latest_tls_version(),
                       next_protocol_fn npn = next_protocol_fn());

      /**
      * Completes full handshake then returns
      */
      void do_handshake();

      /**
      * Number of bytes pending read in the plaintext buffer (bytes
      * readable without blocking)
      */
      size_t pending() const { return m_plaintext.size(); }

      /**
      * Blocking read, will return at least 1 byte or 0 on connection close
      */
      size_t read(byte buf[], size_t buf_len);

      void write(const byte buf[], size_t buf_len) { m_channel.send(buf, buf_len); }

      const TLS::Channel& underlying_channel() const { return m_channel; }
      TLS::Channel& underlying_channel() { return m_channel; }

      void close() { m_channel.close(); }

      bool is_closed() const { return m_channel.is_closed(); }

      std::vector<X509_Certificate> peer_cert_chain() const
         { return m_channel.peer_cert_chain(); }

      virtual ~Blocking_Client() {}

   protected:
      /**
      * Application can override to get the handshake complete notification
      */
      virtual bool handshake_complete(const Session&) { return true; }

      /**
      * Application can override to get notification of alerts
      */
      virtual void alert_notification(const Alert&) {}

   private:

      bool handshake_cb(const Session&);

      void data_cb(const byte data[], size_t data_len);

      void alert_cb(const Alert alert, const byte data[], size_t data_len);

      read_fn m_read;
      TLS::Client m_channel;
      secure_vector<byte> m_plaintext;
   };

}

}


namespace Botan {

namespace TLS {

/**
* Exception Base Class
*/
class BOTAN_DLL TLS_Exception : public Exception
   {
   public:
      Alert::Type type() const { return m_alert_type; }

      TLS_Exception(Alert::Type type,
                    const std::string& err_msg = "Unknown error") :
         Exception(err_msg), m_alert_type(type) {}

   private:
      Alert::Type m_alert_type;
   };

/**
* Unexpected_Message Exception
*/
struct BOTAN_DLL Unexpected_Message : public TLS_Exception
   {
   Unexpected_Message(const std::string& err) :
      TLS_Exception(Alert::UNEXPECTED_MESSAGE, err) {}
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Handshake Message Base Class
*/
class BOTAN_DLL Handshake_Message
   {
   public:
      virtual Handshake_Type type() const = 0;

      virtual std::vector<byte> serialize() const = 0;

      virtual ~Handshake_Message() {}
   };

}

}


namespace Botan {

namespace TLS {

/**
* An implementation of Session_Manager that saves values in a SQL
* database file, with the session data encrypted using a passphrase.
*
* @warning For clients, the hostnames associated with the saved
* sessions are stored in the database in plaintext. This may be a
* serious privacy risk in some situations.
*/
class BOTAN_DLL Session_Manager_SQL : public Session_Manager
   {
   public:
      /**
      * @param db A connection to the database to use
               The table names botan_tls_sessions and
               botan_tls_sessions_metadata will be used
      * @param passphrase used to encrypt the session data
      * @param rng a random number generator
      * @param max_sessions a hint on the maximum number of sessions
      *        to keep in memory at any one time. (If zero, don't cap)
      * @param session_lifetime sessions are expired after this many
      *        seconds have elapsed from initial handshake.
      */
      Session_Manager_SQL(std::shared_ptr<SQL_Database> db,
                          const std::string& passphrase,
                          RandomNumberGenerator& rng,
                          size_t max_sessions = 1000,
                          std::chrono::seconds session_lifetime = std::chrono::seconds(7200));

      Session_Manager_SQL(const Session_Manager_SQL&) = delete;

      Session_Manager_SQL& operator=(const Session_Manager_SQL&) = delete;

      bool load_from_session_id(const std::vector<byte>& session_id,
                                Session& session) override;

      bool load_from_server_info(const Server_Information& info,
                                 Session& session) override;

      void remove_entry(const std::vector<byte>& session_id) override;

      void save(const Session& session_data) override;

      std::chrono::seconds session_lifetime() const override
         { return m_session_lifetime; }

   private:
      void prune_session_cache();

      std::shared_ptr<SQL_Database> m_db;
      SymmetricKey m_session_key;
      RandomNumberGenerator& m_rng;
      size_t m_max_sessions;
      std::chrono::seconds m_session_lifetime;
   };

}

}


namespace Botan {

/**
* Filter interface for Transforms
*/
class BOTAN_DLL Transform_Filter : public Keyed_Filter,
                                   private Buffered_Filter
   {
   public:
      Transform_Filter(Transform* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Transform& get_transform() const { return *m_transform; }

      Transform& get_transform() { return *m_transform; }

   private:
      void write(const byte input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const byte input[], size_t input_length) override;
      void buffered_final(const byte input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<byte> get();
         private:
            bool m_fresh_nonce;
            std::vector<byte> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Transform> m_transform;
      secure_vector<byte> m_buffer;
   };

typedef Transform_Filter Transformation_Filter;

}


namespace Botan {

/**
* A split secret, using the format from draft-mcgrew-tss-03
*/
class BOTAN_DLL RTSS_Share
   {
   public:
      /**
      * @param M the number of shares needed to reconstruct
      * @param N the number of shares generated
      * @param secret the secret to split
      * @param secret_len the length of the secret
      * @param identifier the 16 byte share identifier
      * @param rng the random number generator to use
      */
      static std::vector<RTSS_Share>
         split(byte M, byte N,
               const byte secret[], u16bit secret_len,
               const byte identifier[16],
               RandomNumberGenerator& rng);

      /**
      * @param shares the list of shares
      */
      static secure_vector<byte>
        reconstruct(const std::vector<RTSS_Share>& shares);

      RTSS_Share() {}

      /**
      * @param hex_input the share encoded in hexadecimal
      */
      RTSS_Share(const std::string& hex_input);

      /**
      * @return hex representation
      */
      std::string to_string() const;

      /**
      * @return share identifier
      */
      byte share_id() const;

      /**
      * @return size of this share in bytes
      */
      size_t size() const { return contents.size(); }

      /**
      * @return if this TSS share was initialized or not
      */
      bool initialized() const { return (contents.size() > 0); }
   private:
      secure_vector<byte> contents;
   };

}


namespace Botan {

/**
* Twofish, an AES finalist
*/
class BOTAN_DLL Twofish : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Twofish"; }
      BlockCipher* clone() const { return new Twofish; }
   private:
      void key_schedule(const byte[], size_t);

      static void rs_mul(byte[4], byte, size_t);

      static const u32bit MDS0[256];
      static const u32bit MDS1[256];
      static const u32bit MDS2[256];
      static const u32bit MDS3[256];
      static const byte Q0[256];
      static const byte Q1[256];
      static const byte RS[32];
      static const byte EXP_TO_POLY[255];
      static const byte POLY_TO_EXP[255];

      secure_vector<u32bit> SB, RK;
   };

}


namespace Botan {

/**
* Whirlpool
*/
class BOTAN_DLL Whirlpool : public MDx_HashFunction
   {
   public:
      std::string name() const { return "Whirlpool"; }
      size_t output_length() const { return 64; }
      HashFunction* clone() const { return new Whirlpool; }

      void clear();

      Whirlpool() : MDx_HashFunction(64, true, true, 32), M(8), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      static const u64bit C0[256];
      static const u64bit C1[256];
      static const u64bit C2[256];
      static const u64bit C3[256];
      static const u64bit C4[256];
      static const u64bit C5[256];
      static const u64bit C6[256];
      static const u64bit C7[256];

      secure_vector<u64bit> M, digest;
   };

}


namespace Botan {

/**
* Estimate work factor for discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
size_t dl_work_factor(size_t prime_group_size);

/**
* Estimate work factor for EC discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
size_t ecp_work_factor(size_t prime_group_size);

}


namespace Botan {

/**
* X.509 Certificate Extension
*/
class BOTAN_DLL Certificate_Extension
   {
   public:
      /**
      * @return OID representing this extension
      */
      OID oid_of() const;

      /**
      * Make a copy of this extension
      * @return copy of this
      */
      virtual Certificate_Extension* copy() const = 0;

      /*
      * Add the contents of this extension into the information
      * for the subject and/or issuer, as necessary.
      * @param subject the subject info
      * @param issuer the issuer info
      */
      virtual void contents_to(Data_Store& subject,
                               Data_Store& issuer) const = 0;

      /*
      * @return specific OID name
      */
      virtual std::string oid_name() const = 0;

      virtual ~Certificate_Extension() {}
   protected:
      friend class Extensions;
      virtual bool should_encode() const { return true; }
      virtual std::vector<byte> encode_inner() const = 0;
      virtual void decode_inner(const std::vector<byte>&) = 0;
   };

/**
* X.509 Certificate Extension List
*/
class BOTAN_DLL Extensions : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      void contents_to(Data_Store&, Data_Store&) const;

      void add(Certificate_Extension* extn, bool critical = false);

      Extensions& operator=(const Extensions&);

      Extensions(const Extensions&);
      Extensions(bool st = true) : m_throw_on_unknown_critical(st) {}
      ~Extensions();
   private:
      static Certificate_Extension* get_extension(const OID&);

      std::vector<std::pair<Certificate_Extension*, bool> > extensions;
      bool m_throw_on_unknown_critical;
   };

namespace Cert_Extension {

static const size_t NO_CERT_PATH_LIMIT = 0xFFFFFFF0;

/**
* Basic Constraints Extension
*/
class BOTAN_DLL Basic_Constraints : public Certificate_Extension
   {
   public:
      Basic_Constraints* copy() const
         { return new Basic_Constraints(is_ca, path_limit); }

      Basic_Constraints(bool ca = false, size_t limit = 0) :
         is_ca(ca), path_limit(limit) {}

      bool get_is_ca() const { return is_ca; }
      size_t get_path_limit() const;
   private:
      std::string oid_name() const { return "X509v3.BasicConstraints"; }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      bool is_ca;
      size_t path_limit;
   };

/**
* Key Usage Constraints Extension
*/
class BOTAN_DLL Key_Usage : public Certificate_Extension
   {
   public:
      Key_Usage* copy() const { return new Key_Usage(constraints); }

      Key_Usage(Key_Constraints c = NO_CONSTRAINTS) : constraints(c) {}

      Key_Constraints get_constraints() const { return constraints; }
   private:
      std::string oid_name() const { return "X509v3.KeyUsage"; }

      bool should_encode() const { return (constraints != NO_CONSTRAINTS); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      Key_Constraints constraints;
   };

/**
* Subject Key Identifier Extension
*/
class BOTAN_DLL Subject_Key_ID : public Certificate_Extension
   {
   public:
      Subject_Key_ID* copy() const { return new Subject_Key_ID(key_id); }

      Subject_Key_ID() {}
      Subject_Key_ID(const std::vector<byte>&);

      std::vector<byte> get_key_id() const { return key_id; }
   private:
      std::string oid_name() const { return "X509v3.SubjectKeyIdentifier"; }

      bool should_encode() const { return (key_id.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<byte> key_id;
   };

/**
* Authority Key Identifier Extension
*/
class BOTAN_DLL Authority_Key_ID : public Certificate_Extension
   {
   public:
      Authority_Key_ID* copy() const { return new Authority_Key_ID(key_id); }

      Authority_Key_ID() {}
      Authority_Key_ID(const std::vector<byte>& k) : key_id(k) {}

      std::vector<byte> get_key_id() const { return key_id; }
   private:
      std::string oid_name() const { return "X509v3.AuthorityKeyIdentifier"; }

      bool should_encode() const { return (key_id.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<byte> key_id;
   };

/**
* Alternative Name Extension Base Class
*/
class BOTAN_DLL Alternative_Name : public Certificate_Extension
   {
   public:
      AlternativeName get_alt_name() const { return alt_name; }

   protected:
      Alternative_Name(const AlternativeName&, const std::string& oid_name);

      Alternative_Name(const std::string&, const std::string&);
   private:
      std::string oid_name() const { return oid_name_str; }

      bool should_encode() const { return alt_name.has_items(); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::string oid_name_str;
      AlternativeName alt_name;
   };

/**
* Subject Alternative Name Extension
*/
class BOTAN_DLL Subject_Alternative_Name : public Alternative_Name
   {
   public:
      Subject_Alternative_Name* copy() const
         { return new Subject_Alternative_Name(get_alt_name()); }

      Subject_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Issuer Alternative Name Extension
*/
class BOTAN_DLL Issuer_Alternative_Name : public Alternative_Name
   {
   public:
      Issuer_Alternative_Name* copy() const
         { return new Issuer_Alternative_Name(get_alt_name()); }

      Issuer_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Extended Key Usage Extension
*/
class BOTAN_DLL Extended_Key_Usage : public Certificate_Extension
   {
   public:
      Extended_Key_Usage* copy() const { return new Extended_Key_Usage(oids); }

      Extended_Key_Usage() {}
      Extended_Key_Usage(const std::vector<OID>& o) : oids(o) {}

      std::vector<OID> get_oids() const { return oids; }
   private:
      std::string oid_name() const { return "X509v3.ExtendedKeyUsage"; }

      bool should_encode() const { return (oids.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<OID> oids;
   };

/**
* Certificate Policies Extension
*/
class BOTAN_DLL Certificate_Policies : public Certificate_Extension
   {
   public:
      Certificate_Policies* copy() const
         { return new Certificate_Policies(oids); }

      Certificate_Policies() {}
      Certificate_Policies(const std::vector<OID>& o) : oids(o) {}

      std::vector<OID> get_oids() const { return oids; }
   private:
      std::string oid_name() const { return "X509v3.CertificatePolicies"; }

      bool should_encode() const { return (oids.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<OID> oids;
   };

class BOTAN_DLL Authority_Information_Access : public Certificate_Extension
   {
   public:
      Authority_Information_Access* copy() const
         { return new Authority_Information_Access(m_ocsp_responder); }

      Authority_Information_Access() {}

      Authority_Information_Access(const std::string& ocsp) :
         m_ocsp_responder(ocsp) {}

   private:
      std::string oid_name() const { return "PKIX.AuthorityInformationAccess"; }

      bool should_encode() const { return (m_ocsp_responder != ""); }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);

      void contents_to(Data_Store&, Data_Store&) const;

      std::string m_ocsp_responder;
   };

/**
* CRL Number Extension
*/
class BOTAN_DLL CRL_Number : public Certificate_Extension
   {
   public:
      CRL_Number* copy() const;

      CRL_Number() : has_value(false), crl_number(0) {}
      CRL_Number(size_t n) : has_value(true), crl_number(n) {}

      size_t get_crl_number() const;
   private:
      std::string oid_name() const { return "X509v3.CRLNumber"; }

      bool should_encode() const { return has_value; }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      bool has_value;
      size_t crl_number;
   };

/**
* CRL Entry Reason Code Extension
*/
class BOTAN_DLL CRL_ReasonCode : public Certificate_Extension
   {
   public:
      CRL_ReasonCode* copy() const { return new CRL_ReasonCode(reason); }

      CRL_ReasonCode(CRL_Code r = UNSPECIFIED) : reason(r) {}

      CRL_Code get_reason() const { return reason; }
   private:
      std::string oid_name() const { return "X509v3.ReasonCode"; }

      bool should_encode() const { return (reason != UNSPECIFIED); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      CRL_Code reason;
   };

/**
* CRL Distribution Points Extension
*/
class BOTAN_DLL CRL_Distribution_Points : public Certificate_Extension
   {
   public:
      class BOTAN_DLL Distribution_Point : public ASN1_Object
         {
         public:
            void encode_into(class DER_Encoder&) const;
            void decode_from(class BER_Decoder&);

            const AlternativeName& point() const { return m_point; }
         private:
            AlternativeName m_point;
         };

      CRL_Distribution_Points* copy() const
         { return new CRL_Distribution_Points(m_distribution_points); }

      CRL_Distribution_Points() {}

      CRL_Distribution_Points(const std::vector<Distribution_Point>& points) :
         m_distribution_points(points) {}

      std::vector<Distribution_Point> distribution_points() const
         { return m_distribution_points; }

   private:
      std::string oid_name() const { return "X509v3.CRLDistributionPoints"; }

      bool should_encode() const { return !m_distribution_points.empty(); }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<Distribution_Point> m_distribution_points;
   };

}

}


namespace Botan {

/**
* This class represents X.509 Certificate Authorities (CAs).
*/
class BOTAN_DLL X509_CA
   {
   public:

      /**
      * Sign a PKCS#10 Request.
      * @param req the request to sign
      * @param rng the rng to use
      * @param not_before the starting time for the certificate
      * @param not_after the expiration time for the certificate
      * @return resulting certificate
      */
      X509_Certificate sign_request(const PKCS10_Request& req,
                                    RandomNumberGenerator& rng,
                                    const X509_Time& not_before,
                                    const X509_Time& not_after);

      /**
      * Get the certificate of this CA.
      * @return CA certificate
      */
      X509_Certificate ca_certificate() const;

      /**
      * Create a new and empty CRL for this CA.
      * @param rng the random number generator to use
      * @param next_update the time to set in next update in seconds
      * as the offset from the current time
      * @return new CRL
      */
      X509_CRL new_crl(RandomNumberGenerator& rng,
                       u32bit next_update = 0) const;

      /**
      * Create a new CRL by with additional entries.
      * @param last_crl the last CRL of this CA to add the new entries to
      * @param new_entries contains the new CRL entries to be added to the CRL
      * @param rng the random number generator to use
      * @param next_update the time to set in next update in seconds
      * as the offset from the current time
      */
      X509_CRL update_crl(const X509_CRL& last_crl,
                          const std::vector<CRL_Entry>& new_entries,
                          RandomNumberGenerator& rng,
                          u32bit next_update = 0) const;

      /**
      * Interface for creating new certificates
      * @param signer a signing object
      * @param rng a random number generator
      * @param sig_algo the signature algorithm identifier
      * @param pub_key the serialized public key
      * @param not_before the start time of the certificate
      * @param not_after the end time of the certificate
      * @param issuer_dn the DN of the issuer
      * @param subject_dn the DN of the subject
      * @param extensions an optional list of certificate extensions
      * @returns newly minted certificate
      */
      static X509_Certificate make_cert(PK_Signer* signer,
                                        RandomNumberGenerator& rng,
                                        const AlgorithmIdentifier& sig_algo,
                                        const std::vector<byte>& pub_key,
                                        const X509_Time& not_before,
                                        const X509_Time& not_after,
                                        const X509_DN& issuer_dn,
                                        const X509_DN& subject_dn,
                                        const Extensions& extensions);

      /**
      * Create a new CA object.
      * @param ca_certificate the certificate of the CA
      * @param key the private key of the CA
      * @param hash_fn name of a hash function to use for signing
      */
      X509_CA(const X509_Certificate& ca_certificate,
              const Private_Key& key,
              const std::string& hash_fn);

      X509_CA(const X509_CA&) = delete;
      X509_CA& operator=(const X509_CA&) = delete;

      ~X509_CA();
   private:
      X509_CRL make_crl(const std::vector<CRL_Entry>& entries,
                        u32bit crl_number, u32bit next_update,
                        RandomNumberGenerator& rng) const;

      AlgorithmIdentifier ca_sig_algo;
      X509_Certificate cert;
      PK_Signer* signer;
   };

/**
* Choose the default signature format for a certain public key signature
* scheme.
* @param key will be the key to choose a padding scheme for
* @param hash_fn is the desired hash function
* @param alg_id will be set to the chosen scheme
* @return A PK_Signer object for generating signatures
*/
BOTAN_DLL PK_Signer* choose_sig_format(const Private_Key& key,
                                       const std::string& hash_fn,
                                       AlgorithmIdentifier& alg_id);

}


namespace Botan {

/**
* Specifies restrictions on the PKIX path validation
*/
class BOTAN_DLL Path_Validation_Restrictions
   {
   public:
      /**
      * @param require_rev if true, revocation information is required
      * @param minimum_key_strength is the minimum strength (in terms of
      *        operations, eg 80 means 2^80) of a signature. Signatures
      *        weaker than this are rejected. If more than 80, SHA-1
      *        signatures are also rejected.
      */
      Path_Validation_Restrictions(bool require_rev = false,
                                   size_t minimum_key_strength = 80,
                                   bool ocsp_all_intermediates = false);

      /**
      * @param require_rev if true, revocation information is required
      * @param minimum_key_strength is the minimum strength (in terms of
      *        operations, eg 80 means 2^80) of a signature. Signatures
      *        weaker than this are rejected.
      * @param trusted_hashes a set of trusted hashes. Any signatures
      *        created using a hash other than one of these will be
      *        rejected.
      */
      Path_Validation_Restrictions(bool require_rev,
                                   size_t minimum_key_strength,
                                   bool ocsp_all_intermediates,
                                   const std::set<std::string>& trusted_hashes) :
         m_require_revocation_information(require_rev),
         m_ocsp_all_intermediates(ocsp_all_intermediates),
         m_trusted_hashes(trusted_hashes),
         m_minimum_key_strength(minimum_key_strength) {}

      bool require_revocation_information() const
         { return m_require_revocation_information; }

      bool ocsp_all_intermediates() const
         { return m_ocsp_all_intermediates; }

      const std::set<std::string>& trusted_hashes() const
         { return m_trusted_hashes; }

      size_t minimum_key_strength() const
         { return m_minimum_key_strength; }

   private:
      bool m_require_revocation_information;
      bool m_ocsp_all_intermediates;
      std::set<std::string> m_trusted_hashes;
      size_t m_minimum_key_strength;
   };

/**
* Represents the result of a PKIX path validation
*/
class BOTAN_DLL Path_Validation_Result
   {
   public:
      typedef Certificate_Status_Code Code;

      /**
      * @return the set of hash functions you are implicitly
      * trusting by trusting this result.
      */
      std::set<std::string> trusted_hashes() const;

      /**
      * @return the trust root of the validation
      */
      const X509_Certificate& trust_root() const;

      /**
      * @return the full path from subject to trust root
      */
      const std::vector<X509_Certificate>& cert_path() const { return m_cert_path; }

      /**
      * @return true iff the validation was succesful
      */
      bool successful_validation() const;

      /**
      * @return overall validation result code
      */
      Certificate_Status_Code result() const { return m_overall; }

      /**
      * Return a set of status codes for each certificate in the chain
      */
      const std::vector<std::set<Certificate_Status_Code>>& all_statuses() const
         { return m_all_status; }

      /**
      * @return string representation of the validation result
      */
      std::string result_string() const;

      static const char* status_string(Certificate_Status_Code code);

      Path_Validation_Result(std::vector<std::set<Certificate_Status_Code>> status,
                             std::vector<X509_Certificate>&& cert_chain);

      Path_Validation_Result(Certificate_Status_Code status) : m_overall(status) {}

   private:
      friend Path_Validation_Result BOTAN_DLL x509_path_validate(
         const std::vector<X509_Certificate>& end_certs,
         const Path_Validation_Restrictions& restrictions,
         const std::vector<Certificate_Store*>& certstores);

      Certificate_Status_Code m_overall;
      std::vector<std::set<Certificate_Status_Code>> m_all_status;
      std::vector<X509_Certificate> m_cert_path;
   };

/**
* PKIX Path Validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const std::vector<X509_Certificate>& end_certs,
   const Path_Validation_Restrictions& restrictions,
   const std::vector<Certificate_Store*>& certstores);

/**
* PKIX Path Validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const X509_Certificate& end_cert,
   const Path_Validation_Restrictions& restrictions,
   const std::vector<Certificate_Store*>& certstores);

/**
* PKIX Path Validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const X509_Certificate& end_cert,
   const Path_Validation_Restrictions& restrictions,
   const Certificate_Store& store);

/**
* PKIX Path Validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const std::vector<X509_Certificate>& end_certs,
   const Path_Validation_Restrictions& restrictions,
   const Certificate_Store& store);

}


namespace Botan {

/**
* Options for X.509 certificates.
*/
class BOTAN_DLL X509_Cert_Options
   {
   public:
      /**
      * the subject common name
      */
      std::string common_name;

      /**
      * the subject counry
      */
      std::string country;

      /**
      * the subject organization
      */
      std::string organization;

      /**
      * the subject organizational unit
      */
      std::string org_unit;

      /**
      * the subject locality
      */
      std::string locality;

      /**
      * the subject state
      */
      std::string state;

      /**
      * the subject serial number
      */
      std::string serial_number;

      /**
      * the subject email adress
      */
      std::string email;

      /**
      * the subject URI
      */
      std::string uri;

      /**
      * the subject IPv4 address
      */
      std::string ip;

      /**
      * the subject DNS
      */
      std::string dns;

      /**
      * the subject XMPP
      */
      std::string xmpp;

      /**
      * the subject challenge password
      */
      std::string challenge;

      /**
      * the subject notBefore
      */
      X509_Time start;
      /**
      * the subject notAfter
      */
      X509_Time end;

      /**
      * Indicates whether the certificate request
      */
      bool is_CA;

      /**
      * Indicates the BasicConstraints path limit
      */
      size_t path_limit;

      /**
      * The key constraints for the subject public key
      */
      Key_Constraints constraints;

      /**
      * The key extended constraints for the subject public key
      */
      std::vector<OID> ex_constraints;

      /**
      * Check the options set in this object for validity.
      */
      void sanity_check() const;

      /**
      * Mark the certificate as a CA certificate and set the path limit.
      * @param limit the path limit to be set in the BasicConstraints extension.
      */
      void CA_key(size_t limit = 1);

      /**
      * Set the notBefore of the certificate.
      * @param time the notBefore value of the certificate
      */
      void not_before(const std::string& time);

      /**
      * Set the notAfter of the certificate.
      * @param time the notAfter value of the certificate
      */
      void not_after(const std::string& time);

      /**
      * Add the key constraints of the KeyUsage extension.
      * @param constr the constraints to set
      */
      void add_constraints(Key_Constraints constr);

      /**
      * Add constraints to the ExtendedKeyUsage extension.
      * @param oid the oid to add
      */
      void add_ex_constraint(const OID& oid);

      /**
      * Add constraints to the ExtendedKeyUsage extension.
      * @param name the name to look up the oid to add
      */
      void add_ex_constraint(const std::string& name);

      /**
      * Construct a new options object
      * @param opts define the common name of this object. An example for this
      * parameter would be "common_name/country/organization/organizational_unit".
      * @param expire_time the expiration time (from the current clock in seconds)
      */
      X509_Cert_Options(const std::string& opts = "",
                        u32bit expire_time = 365 * 24 * 60 * 60);
   };

namespace X509 {

/**
* Create a self-signed X.509 certificate.
* @param opts the options defining the certificate to create
* @param key the private key used for signing, i.e. the key
* associated with this self-signed certificate
* @param hash_fn the hash function to use
* @param rng the rng to use
* @return newly created self-signed certificate
*/
BOTAN_DLL X509_Certificate
create_self_signed_cert(const X509_Cert_Options& opts,
                        const Private_Key& key,
                        const std::string& hash_fn,
                        RandomNumberGenerator& rng);

/**
* Create a PKCS#10 certificate request.
* @param opts the options defining the request to create
* @param key the key used to sign this request
* @param rng the rng to use
* @param hash_fn the hash function to use
* @return newly created PKCS#10 request
*/
BOTAN_DLL PKCS10_Request create_cert_req(const X509_Cert_Options& opts,
                                         const Private_Key& key,
                                         const std::string& hash_fn,
                                         RandomNumberGenerator& rng);

}

}


namespace Botan {

/**
* DES/3DES-based MAC from ANSI X9.19
*/
class BOTAN_DLL ANSI_X919_MAC : public MessageAuthenticationCode
   {
   public:
      void clear();
      std::string name() const;
      size_t output_length() const { return 8; }

      MessageAuthenticationCode* clone() const;

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(8, 16, 8);
         }

      ANSI_X919_MAC();

      ANSI_X919_MAC(const ANSI_X919_MAC&) = delete;
      ANSI_X919_MAC& operator=(const ANSI_X919_MAC&) = delete;
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      std::unique_ptr<BlockCipher> m_des1, m_des2;
      secure_vector<byte> m_state;
      size_t m_position;
   };

}


namespace Botan {

/**
* ANSI X9.31 RNG
*/
class BOTAN_DLL ANSI_X931_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte[], size_t);
      bool is_seeded() const;
      void clear();
      std::string name() const;

      void reseed(size_t poll_bits);
      void add_entropy(const byte[], size_t);

      /**
      * @param cipher the block cipher to use in this PRNG
      * @param rng the underlying PRNG for generating inputs
      * (eg, an HMAC_RNG)
      */
      ANSI_X931_RNG(BlockCipher* cipher,
                    RandomNumberGenerator* rng);

   private:
      void rekey();
      void update_buffer();

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<RandomNumberGenerator> m_prng;
      secure_vector<byte> m_V, m_R;
      size_t m_R_pos;
   };

}


namespace Botan {

/**
* XTEA
*/
class BOTAN_DLL XTEA : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "XTEA"; }
      BlockCipher* clone() const { return new XTEA; }
   protected:
      /**
      * @return const reference to the key schedule
      */
      const secure_vector<u32bit>& get_EK() const { return EK; }

   private:
      void key_schedule(const byte[], size_t);
      secure_vector<u32bit> EK;
   };

}


namespace Botan {

/**
* XTEA implemented using SIMD operations
*/
class BOTAN_DLL XTEA_SIMD : public XTEA
   {
   public:
      size_t parallelism() const { return 8; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;
      BlockCipher* clone() const { return new XTEA_SIMD; }
   };

}


namespace Botan {

/**
* IEEE P1619 XTS Mode
*/
class BOTAN_DLL XTS_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      XTS_Mode(BlockCipher* cipher);

      const byte* tweak() const { return &m_tweak[0]; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void update_tweak(size_t last_used);

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;
      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher, m_tweak_cipher;
      secure_vector<byte> m_tweak;
   };

/**
* IEEE P1619 XTS Encryption
*/
class BOTAN_DLL XTS_Encryption : public XTS_Mode
   {
   public:
      XTS_Encryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

/**
* IEEE P1619 XTS Decryption
*/
class BOTAN_DLL XTS_Decryption : public XTS_Mode
   {
   public:
      XTS_Decryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

}


#endif

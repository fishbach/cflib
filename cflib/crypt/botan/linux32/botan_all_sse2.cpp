/*
* Botan 1.11.16 Amalgamation
* (C) 1999-2013,2014,2015 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

/*
* IDEA in SSE2
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <emmintrin.h>

namespace Botan {

BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_sse2(), IDEA_SSE2, "IDEA", "sse2", 64);

namespace {

inline __m128i mul(__m128i X, u16bit K_16)
   {
   const __m128i zeros = _mm_set1_epi16(0);
   const __m128i ones = _mm_set1_epi16(1);

   const __m128i K = _mm_set1_epi16(K_16);

   const __m128i X_is_zero = _mm_cmpeq_epi16(X, zeros);
   const __m128i K_is_zero = _mm_cmpeq_epi16(K, zeros);

   const __m128i mul_lo = _mm_mullo_epi16(X, K);
   const __m128i mul_hi = _mm_mulhi_epu16(X, K);

   __m128i T = _mm_sub_epi16(mul_lo, mul_hi);

   // Unsigned compare; cmp = 1 if mul_lo < mul_hi else 0
   const __m128i subs = _mm_subs_epu16(mul_hi, mul_lo);
   const __m128i cmp = _mm_min_epu8(
     _mm_or_si128(subs, _mm_srli_epi16(subs, 8)), ones);

   T = _mm_add_epi16(T, cmp);

   /* Selection: if X[i] is zero then assign 1-K
                 if K is zero then assign 1-X[i]

      Could if() off value of K_16 for the second, but this gives a
      constant time implementation which is a nice bonus.
   */

   T = _mm_or_si128(
      _mm_andnot_si128(X_is_zero, T),
      _mm_and_si128(_mm_sub_epi16(ones, K), X_is_zero));

   T = _mm_or_si128(
      _mm_andnot_si128(K_is_zero, T),
      _mm_and_si128(_mm_sub_epi16(ones, X), K_is_zero));

   return T;
   }

/*
* 4x8 matrix transpose
*
* FIXME: why do I need the extra set of unpack_epi32 here? Inverse in
* transpose_out doesn't need it. Something with the shuffle? Removing
* that extra unpack could easily save 3-4 cycles per block, and would
* also help a lot with register pressure on 32-bit x86
*/
void transpose_in(__m128i& B0, __m128i& B1, __m128i& B2, __m128i& B3)
   {
   __m128i T0 = _mm_unpackhi_epi32(B0, B1);
   __m128i T1 = _mm_unpacklo_epi32(B0, B1);
   __m128i T2 = _mm_unpackhi_epi32(B2, B3);
   __m128i T3 = _mm_unpacklo_epi32(B2, B3);

   __m128i T4 = _mm_unpacklo_epi32(T0, T1);
   __m128i T5 = _mm_unpackhi_epi32(T0, T1);
   __m128i T6 = _mm_unpacklo_epi32(T2, T3);
   __m128i T7 = _mm_unpackhi_epi32(T2, T3);

   T0 = _mm_shufflehi_epi16(T4, _MM_SHUFFLE(1, 3, 0, 2));
   T1 = _mm_shufflehi_epi16(T5, _MM_SHUFFLE(1, 3, 0, 2));
   T2 = _mm_shufflehi_epi16(T6, _MM_SHUFFLE(1, 3, 0, 2));
   T3 = _mm_shufflehi_epi16(T7, _MM_SHUFFLE(1, 3, 0, 2));

   T0 = _mm_shufflelo_epi16(T0, _MM_SHUFFLE(1, 3, 0, 2));
   T1 = _mm_shufflelo_epi16(T1, _MM_SHUFFLE(1, 3, 0, 2));
   T2 = _mm_shufflelo_epi16(T2, _MM_SHUFFLE(1, 3, 0, 2));
   T3 = _mm_shufflelo_epi16(T3, _MM_SHUFFLE(1, 3, 0, 2));

   T0 = _mm_shuffle_epi32(T0, _MM_SHUFFLE(3, 1, 2, 0));
   T1 = _mm_shuffle_epi32(T1, _MM_SHUFFLE(3, 1, 2, 0));
   T2 = _mm_shuffle_epi32(T2, _MM_SHUFFLE(3, 1, 2, 0));
   T3 = _mm_shuffle_epi32(T3, _MM_SHUFFLE(3, 1, 2, 0));

   B0 = _mm_unpacklo_epi64(T0, T2);
   B1 = _mm_unpackhi_epi64(T0, T2);
   B2 = _mm_unpacklo_epi64(T1, T3);
   B3 = _mm_unpackhi_epi64(T1, T3);
   }

/*
* 4x8 matrix transpose (reverse)
*/
void transpose_out(__m128i& B0, __m128i& B1, __m128i& B2, __m128i& B3)
   {
   __m128i T0 = _mm_unpacklo_epi64(B0, B1);
   __m128i T1 = _mm_unpacklo_epi64(B2, B3);
   __m128i T2 = _mm_unpackhi_epi64(B0, B1);
   __m128i T3 = _mm_unpackhi_epi64(B2, B3);

   T0 = _mm_shuffle_epi32(T0, _MM_SHUFFLE(3, 1, 2, 0));
   T1 = _mm_shuffle_epi32(T1, _MM_SHUFFLE(3, 1, 2, 0));
   T2 = _mm_shuffle_epi32(T2, _MM_SHUFFLE(3, 1, 2, 0));
   T3 = _mm_shuffle_epi32(T3, _MM_SHUFFLE(3, 1, 2, 0));

   T0 = _mm_shufflehi_epi16(T0, _MM_SHUFFLE(3, 1, 2, 0));
   T1 = _mm_shufflehi_epi16(T1, _MM_SHUFFLE(3, 1, 2, 0));
   T2 = _mm_shufflehi_epi16(T2, _MM_SHUFFLE(3, 1, 2, 0));
   T3 = _mm_shufflehi_epi16(T3, _MM_SHUFFLE(3, 1, 2, 0));

   T0 = _mm_shufflelo_epi16(T0, _MM_SHUFFLE(3, 1, 2, 0));
   T1 = _mm_shufflelo_epi16(T1, _MM_SHUFFLE(3, 1, 2, 0));
   T2 = _mm_shufflelo_epi16(T2, _MM_SHUFFLE(3, 1, 2, 0));
   T3 = _mm_shufflelo_epi16(T3, _MM_SHUFFLE(3, 1, 2, 0));

   B0 = _mm_unpacklo_epi32(T0, T1);
   B1 = _mm_unpackhi_epi32(T0, T1);
   B2 = _mm_unpacklo_epi32(T2, T3);
   B3 = _mm_unpackhi_epi32(T2, T3);
   }

/*
* IDEA encryption/decryption in SSE2
*/
void idea_op_8(const byte in[64], byte out[64], const u16bit EK[52])
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);

   __m128i B0 = _mm_loadu_si128(in_mm + 0);
   __m128i B1 = _mm_loadu_si128(in_mm + 1);
   __m128i B2 = _mm_loadu_si128(in_mm + 2);
   __m128i B3 = _mm_loadu_si128(in_mm + 3);

   transpose_in(B0, B1, B2, B3);

   // byte swap
   B0 = _mm_or_si128(_mm_slli_epi16(B0, 8), _mm_srli_epi16(B0, 8));
   B1 = _mm_or_si128(_mm_slli_epi16(B1, 8), _mm_srli_epi16(B1, 8));
   B2 = _mm_or_si128(_mm_slli_epi16(B2, 8), _mm_srli_epi16(B2, 8));
   B3 = _mm_or_si128(_mm_slli_epi16(B3, 8), _mm_srli_epi16(B3, 8));

   for(size_t i = 0; i != 8; ++i)
      {
      B0 = mul(B0, EK[6*i+0]);
      B1 = _mm_add_epi16(B1, _mm_set1_epi16(EK[6*i+1]));
      B2 = _mm_add_epi16(B2, _mm_set1_epi16(EK[6*i+2]));
      B3 = mul(B3, EK[6*i+3]);

      __m128i T0 = B2;

      B2 = _mm_xor_si128(B2, B0);
      B2 = mul(B2, EK[6*i+4]);

      __m128i T1 = B1;

      B1 = _mm_xor_si128(B1, B3);
      B1 = _mm_add_epi16(B1, B2);
      B1 = mul(B1, EK[6*i+5]);

      B2 = _mm_add_epi16(B2, B1);

      B0 = _mm_xor_si128(B0, B1);
      B1 = _mm_xor_si128(B1, T0);
      B3 = _mm_xor_si128(B3, B2);
      B2 = _mm_xor_si128(B2, T1);
      }

   B0 = mul(B0, EK[48]);
   B1 = _mm_add_epi16(B1, _mm_set1_epi16(EK[50]));
   B2 = _mm_add_epi16(B2, _mm_set1_epi16(EK[49]));
   B3 = mul(B3, EK[51]);

   // byte swap
   B0 = _mm_or_si128(_mm_slli_epi16(B0, 8), _mm_srli_epi16(B0, 8));
   B1 = _mm_or_si128(_mm_slli_epi16(B1, 8), _mm_srli_epi16(B1, 8));
   B2 = _mm_or_si128(_mm_slli_epi16(B2, 8), _mm_srli_epi16(B2, 8));
   B3 = _mm_or_si128(_mm_slli_epi16(B3, 8), _mm_srli_epi16(B3, 8));

   transpose_out(B0, B2, B1, B3);

   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   _mm_storeu_si128(out_mm + 0, B0);
   _mm_storeu_si128(out_mm + 1, B2);
   _mm_storeu_si128(out_mm + 2, B1);
   _mm_storeu_si128(out_mm + 3, B3);
   }

}

/*
* IDEA Encryption
*/
void IDEA_SSE2::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u16bit* KS = &this->get_EK()[0];

   while(blocks >= 8)
      {
      idea_op_8(in, out, KS);
      in += 8 * BLOCK_SIZE;
      out += 8 * BLOCK_SIZE;
      blocks -= 8;
      }

   if(blocks)
     IDEA::encrypt_n(in, out, blocks);
   }

/*
* IDEA Decryption
*/
void IDEA_SSE2::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u16bit* KS = &this->get_DK()[0];

   while(blocks >= 8)
      {
      idea_op_8(in, out, KS);
      in += 8 * BLOCK_SIZE;
      out += 8 * BLOCK_SIZE;
      blocks -= 8;
      }

   if(blocks)
     IDEA::decrypt_n(in, out, blocks);
   }

}
/*
* Noekeon in SIMD
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(SIMD_32::enabled(), Noekeon_SIMD, "Noekeon", "simd32", 64);

/*
* Noekeon's Theta Operation
*/
#define NOK_SIMD_THETA(A0, A1, A2, A3, K0, K1, K2, K3)  \
   do {                                                 \
      SIMD_32 T = A0 ^ A2;                              \
      SIMD_32 T_l8 = T;                                 \
      SIMD_32 T_r8 = T;                                 \
      T_l8.rotate_left(8);                              \
      T_r8.rotate_right(8);                             \
      T ^= T_l8;                                        \
      T ^= T_r8;                                        \
      A1 ^= T;                                          \
      A3 ^= T;                                          \
                                                        \
      A0 ^= K0;                                         \
      A1 ^= K1;                                         \
      A2 ^= K2;                                         \
      A3 ^= K3;                                         \
                                                        \
      T = A1 ^ A3;                                      \
      T_l8 = T;                                         \
      T_r8 = T;                                         \
      T_l8.rotate_left(8);                              \
      T_r8.rotate_right(8);                             \
      T ^= T_l8;                                        \
      T ^= T_r8;                                        \
      A0 ^= T;                                          \
      A2 ^= T;                                          \
      } while(0)

/*
* Noekeon's Gamma S-Box Layer
*/
#define NOK_SIMD_GAMMA(A0, A1, A2, A3)                                  \
   do                                                                   \
      {                                                                 \
      A1 ^= A3.andc(~A2);                                               \
      A0 ^= A2 & A1;                                                    \
                                                                        \
      SIMD_32 T = A3;                                                   \
      A3 = A0;                                                          \
      A0 = T;                                                           \
                                                                        \
      A2 ^= A0 ^ A1 ^ A3;                                               \
                                                                        \
      A1 ^= A3.andc(~A2);                                               \
      A0 ^= A2 & A1;                                                    \
      } while(0)

/*
* Noekeon Encryption
*/
void Noekeon_SIMD::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const secure_vector<u32bit>& EK = this->get_EK();

   SIMD_32 K0 = SIMD_32(EK[0]);
   SIMD_32 K1 = SIMD_32(EK[1]);
   SIMD_32 K2 = SIMD_32(EK[2]);
   SIMD_32 K3 = SIMD_32(EK[3]);

   while(blocks >= 4)
      {
      SIMD_32 A0 = SIMD_32::load_be(in     );
      SIMD_32 A1 = SIMD_32::load_be(in + 16);
      SIMD_32 A2 = SIMD_32::load_be(in + 32);
      SIMD_32 A3 = SIMD_32::load_be(in + 48);

      SIMD_32::transpose(A0, A1, A2, A3);

      for(size_t i = 0; i != 16; ++i)
         {
         A0 ^= SIMD_32(RC[i]);

         NOK_SIMD_THETA(A0, A1, A2, A3, K0, K1, K2, K3);

         A1.rotate_left(1);
         A2.rotate_left(5);
         A3.rotate_left(2);

         NOK_SIMD_GAMMA(A0, A1, A2, A3);

         A1.rotate_right(1);
         A2.rotate_right(5);
         A3.rotate_right(2);
         }

      A0 ^= SIMD_32(RC[16]);
      NOK_SIMD_THETA(A0, A1, A2, A3, K0, K1, K2, K3);

      SIMD_32::transpose(A0, A1, A2, A3);

      A0.store_be(out);
      A1.store_be(out + 16);
      A2.store_be(out + 32);
      A3.store_be(out + 48);

      in += 64;
      out += 64;
      blocks -= 4;
      }

   if(blocks)
     Noekeon::encrypt_n(in, out, blocks);
   }

/*
* Noekeon Encryption
*/
void Noekeon_SIMD::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const secure_vector<u32bit>& DK = this->get_DK();

   SIMD_32 K0 = SIMD_32(DK[0]);
   SIMD_32 K1 = SIMD_32(DK[1]);
   SIMD_32 K2 = SIMD_32(DK[2]);
   SIMD_32 K3 = SIMD_32(DK[3]);

   while(blocks >= 4)
      {
      SIMD_32 A0 = SIMD_32::load_be(in     );
      SIMD_32 A1 = SIMD_32::load_be(in + 16);
      SIMD_32 A2 = SIMD_32::load_be(in + 32);
      SIMD_32 A3 = SIMD_32::load_be(in + 48);

      SIMD_32::transpose(A0, A1, A2, A3);

      for(size_t i = 0; i != 16; ++i)
         {
         NOK_SIMD_THETA(A0, A1, A2, A3, K0, K1, K2, K3);

         A0 ^= SIMD_32(RC[16-i]);

         A1.rotate_left(1);
         A2.rotate_left(5);
         A3.rotate_left(2);

         NOK_SIMD_GAMMA(A0, A1, A2, A3);

         A1.rotate_right(1);
         A2.rotate_right(5);
         A3.rotate_right(2);
         }

      NOK_SIMD_THETA(A0, A1, A2, A3, K0, K1, K2, K3);
      A0 ^= SIMD_32(RC[0]);

      SIMD_32::transpose(A0, A1, A2, A3);

      A0.store_be(out);
      A1.store_be(out + 16);
      A2.store_be(out + 32);
      A3.store_be(out + 48);

      in += 64;
      out += 64;
      blocks -= 4;
      }

   if(blocks)
     Noekeon::decrypt_n(in, out, blocks);
   }

}
/*
* Serpent (SIMD)
* (C) 2009,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(SIMD_32::enabled(), Serpent_SIMD, "Serpent", "simd32", 64);

namespace {

#define key_xor(round, B0, B1, B2, B3)                             \
   do {                                                            \
      B0 ^= SIMD_32(keys[4*round  ]);                              \
      B1 ^= SIMD_32(keys[4*round+1]);                              \
      B2 ^= SIMD_32(keys[4*round+2]);                              \
      B3 ^= SIMD_32(keys[4*round+3]);                              \
   } while(0);

/*
* Serpent's linear transformations
*/
#define transform(B0, B1, B2, B3)                                  \
   do {                                                            \
      B0.rotate_left(13);                                          \
      B2.rotate_left(3);                                           \
      B1 ^= B0 ^ B2;                                               \
      B3 ^= B2 ^ (B0 << 3);                                        \
      B1.rotate_left(1);                                           \
      B3.rotate_left(7);                                           \
      B0 ^= B1 ^ B3;                                               \
      B2 ^= B3 ^ (B1 << 7);                                        \
      B0.rotate_left(5);                                           \
      B2.rotate_left(22);                                          \
   } while(0);

#define i_transform(B0, B1, B2, B3)                                \
   do {                                                            \
      B2.rotate_right(22);                                         \
      B0.rotate_right(5);                                          \
      B2 ^= B3 ^ (B1 << 7);                                        \
      B0 ^= B1 ^ B3;                                               \
      B3.rotate_right(7);                                          \
      B1.rotate_right(1);                                          \
      B3 ^= B2 ^ (B0 << 3);                                        \
      B1 ^= B0 ^ B2;                                               \
      B2.rotate_right(3);                                          \
      B0.rotate_right(13);                                         \
   } while(0);

/*
* SIMD Serpent Encryption of 4 blocks in parallel
*/
void serpent_encrypt_4(const byte in[64],
                       byte out[64],
                       const u32bit keys[132])
   {
   SIMD_32 B0 = SIMD_32::load_le(in);
   SIMD_32 B1 = SIMD_32::load_le(in + 16);
   SIMD_32 B2 = SIMD_32::load_le(in + 32);
   SIMD_32 B3 = SIMD_32::load_le(in + 48);

   SIMD_32::transpose(B0, B1, B2, B3);

   key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

   SIMD_32::transpose(B0, B1, B2, B3);

   B0.store_le(out);
   B1.store_le(out + 16);
   B2.store_le(out + 32);
   B3.store_le(out + 48);
   }

/*
* SIMD Serpent Decryption of 4 blocks in parallel
*/
void serpent_decrypt_4(const byte in[64],
                       byte out[64],
                       const u32bit keys[132])
   {
   SIMD_32 B0 = SIMD_32::load_le(in);
   SIMD_32 B1 = SIMD_32::load_le(in + 16);
   SIMD_32 B2 = SIMD_32::load_le(in + 32);
   SIMD_32 B3 = SIMD_32::load_le(in + 48);

   SIMD_32::transpose(B0, B1, B2, B3);

   key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

   SIMD_32::transpose(B0, B1, B2, B3);

   B0.store_le(out);
   B1.store_le(out + 16);
   B2.store_le(out + 32);
   B3.store_le(out + 48);
   }

}

#undef key_xor
#undef transform
#undef i_transform

/*
* Serpent Encryption
*/
void Serpent_SIMD::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_round_keys()[0]);

   while(blocks >= 4)
      {
      serpent_encrypt_4(in, out, KS);
      in += 4 * BLOCK_SIZE;
      out += 4 * BLOCK_SIZE;
      blocks -= 4;
      }

   if(blocks)
     Serpent::encrypt_n(in, out, blocks);
   }

/*
* Serpent Decryption
*/
void Serpent_SIMD::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_round_keys()[0]);

   while(blocks >= 4)
      {
      serpent_decrypt_4(in, out, KS);
      in += 4 * BLOCK_SIZE;
      out += 4 * BLOCK_SIZE;
      blocks -= 4;
      }

   if(blocks)
     Serpent::decrypt_n(in, out, blocks);
   }

}
/*
* SHA-1 using SSE2
* Based on public domain code by Dean Gaudet
*    (http://arctic.org/~dean/crypto/sha1.html)
* (C) 2009-2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

BOTAN_REGISTER_HASH_NOARGS_IF(CPUID::has_sse2(), SHA_160_SSE2, "SHA-160", "sse2", 64);

namespace SHA1_SSE2_F {

namespace {

/*
* First 16 bytes just need byte swapping. Preparing just means
* adding in the round constants.
*/

#define prep00_15(P, W)                                      \
   do {                                                      \
      W = _mm_shufflehi_epi16(W, _MM_SHUFFLE(2, 3, 0, 1));   \
      W = _mm_shufflelo_epi16(W, _MM_SHUFFLE(2, 3, 0, 1));   \
      W = _mm_or_si128(_mm_slli_epi16(W, 8),                 \
                       _mm_srli_epi16(W, 8));                \
      P.u128 = _mm_add_epi32(W, K00_19);                     \
   } while(0)

/*
For each multiple of 4, t, we want to calculate this:

W[t+0] = rol(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
W[t+1] = rol(W[t-2] ^ W[t-7] ^ W[t-13] ^ W[t-15], 1);
W[t+2] = rol(W[t-1] ^ W[t-6] ^ W[t-12] ^ W[t-14], 1);
W[t+3] = rol(W[t]   ^ W[t-5] ^ W[t-11] ^ W[t-13], 1);

we'll actually calculate this:

W[t+0] = rol(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
W[t+1] = rol(W[t-2] ^ W[t-7] ^ W[t-13] ^ W[t-15], 1);
W[t+2] = rol(W[t-1] ^ W[t-6] ^ W[t-12] ^ W[t-14], 1);
W[t+3] = rol(  0    ^ W[t-5] ^ W[t-11] ^ W[t-13], 1);
W[t+3] ^= rol(W[t+0], 1);

the parameters are:

W0 = &W[t-16];
W1 = &W[t-12];
W2 = &W[t- 8];
W3 = &W[t- 4];

and on output:
prepared = W0 + K
W0 = W[t]..W[t+3]
*/

/* note that there is a step here where i want to do a rol by 1, which
* normally would look like this:
*
* r1 = psrld r0,$31
* r0 = pslld r0,$1
* r0 = por r0,r1
*
* but instead i do this:
*
* r1 = pcmpltd r0,zero
* r0 = paddd r0,r0
* r0 = psub r0,r1
*
* because pcmpltd and paddd are availabe in both MMX units on
* efficeon, pentium-m, and opteron but shifts are available in
* only one unit.
*/
#define prep(prep, XW0, XW1, XW2, XW3, K)                               \
   do {                                                                 \
      __m128i r0, r1, r2, r3;                                           \
                                                                        \
      /* load W[t-4] 16-byte aligned, and shift */                      \
      r3 = _mm_srli_si128((XW3), 4);                                    \
      r0 = (XW0);                                                       \
      /* get high 64-bits of XW0 into low 64-bits */                    \
      r1 = _mm_shuffle_epi32((XW0), _MM_SHUFFLE(1,0,3,2));              \
      /* load high 64-bits of r1 */                                     \
      r1 = _mm_unpacklo_epi64(r1, (XW1));                               \
      r2 = (XW2);                                                       \
                                                                        \
      r0 = _mm_xor_si128(r1, r0);                                       \
      r2 = _mm_xor_si128(r3, r2);                                       \
      r0 = _mm_xor_si128(r2, r0);                                       \
      /* unrotated W[t]..W[t+2] in r0 ... still need W[t+3] */          \
                                                                        \
      r2 = _mm_slli_si128(r0, 12);                                      \
      r1 = _mm_cmplt_epi32(r0, _mm_setzero_si128());                    \
      r0 = _mm_add_epi32(r0, r0);   /* shift left by 1 */               \
      r0 = _mm_sub_epi32(r0, r1);   /* r0 has W[t]..W[t+2] */           \
                                                                        \
      r3 = _mm_srli_epi32(r2, 30);                                      \
      r2 = _mm_slli_epi32(r2, 2);                                       \
                                                                        \
      r0 = _mm_xor_si128(r0, r3);                                       \
      r0 = _mm_xor_si128(r0, r2);   /* r0 now has W[t+3] */             \
                                                                        \
      (XW0) = r0;                                                       \
      (prep).u128 = _mm_add_epi32(r0, K);                               \
   } while(0)

/*
* SHA-160 F1 Function
*/
inline void F1(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (D ^ (B & (C ^ D))) + msg + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F2 Function
*/
inline void F2(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (B ^ C ^ D) + msg + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F3 Function
*/
inline void F3(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += ((B & C) | ((B | C) & D)) + msg + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F4 Function
*/
inline void F4(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (B ^ C ^ D) + msg + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

}

}

/*
* SHA-160 Compression Function using SSE for message expansion
*/
void SHA_160_SSE2::compress_n(const byte input_bytes[], size_t blocks)
   {
   using namespace SHA1_SSE2_F;

   const __m128i K00_19 = _mm_set1_epi32(0x5A827999);
   const __m128i K20_39 = _mm_set1_epi32(0x6ED9EBA1);
   const __m128i K40_59 = _mm_set1_epi32(0x8F1BBCDC);
   const __m128i K60_79 = _mm_set1_epi32(0xCA62C1D6);

   u32bit A = digest[0],
          B = digest[1],
          C = digest[2],
          D = digest[3],
          E = digest[4];

   const __m128i* input = reinterpret_cast<const __m128i*>(input_bytes);

   for(size_t i = 0; i != blocks; ++i)
      {
      union v4si {
         u32bit u32[4];
         __m128i u128;
         };

      v4si P0, P1, P2, P3;

      __m128i W0 = _mm_loadu_si128(&input[0]);
      prep00_15(P0, W0);

      __m128i W1 = _mm_loadu_si128(&input[1]);
      prep00_15(P1, W1);

      __m128i W2 = _mm_loadu_si128(&input[2]);
      prep00_15(P2, W2);

      __m128i W3 = _mm_loadu_si128(&input[3]);
      prep00_15(P3, W3);

      /*
      Using SSE4; slower on Core2 and Nehalem
      #define GET_P_32(P, i) _mm_extract_epi32(P.u128, i)

      Much slower on all tested platforms
      #define GET_P_32(P,i) _mm_cvtsi128_si32(_mm_srli_si128(P.u128, i*4))
      */

#define GET_P_32(P, i) P.u32[i]

      F1(A, B, C, D, E, GET_P_32(P0, 0));
      F1(E, A, B, C, D, GET_P_32(P0, 1));
      F1(D, E, A, B, C, GET_P_32(P0, 2));
      F1(C, D, E, A, B, GET_P_32(P0, 3));
      prep(P0, W0, W1, W2, W3, K00_19);

      F1(B, C, D, E, A, GET_P_32(P1, 0));
      F1(A, B, C, D, E, GET_P_32(P1, 1));
      F1(E, A, B, C, D, GET_P_32(P1, 2));
      F1(D, E, A, B, C, GET_P_32(P1, 3));
      prep(P1, W1, W2, W3, W0, K20_39);

      F1(C, D, E, A, B, GET_P_32(P2, 0));
      F1(B, C, D, E, A, GET_P_32(P2, 1));
      F1(A, B, C, D, E, GET_P_32(P2, 2));
      F1(E, A, B, C, D, GET_P_32(P2, 3));
      prep(P2, W2, W3, W0, W1, K20_39);

      F1(D, E, A, B, C, GET_P_32(P3, 0));
      F1(C, D, E, A, B, GET_P_32(P3, 1));
      F1(B, C, D, E, A, GET_P_32(P3, 2));
      F1(A, B, C, D, E, GET_P_32(P3, 3));
      prep(P3, W3, W0, W1, W2, K20_39);

      F1(E, A, B, C, D, GET_P_32(P0, 0));
      F1(D, E, A, B, C, GET_P_32(P0, 1));
      F1(C, D, E, A, B, GET_P_32(P0, 2));
      F1(B, C, D, E, A, GET_P_32(P0, 3));
      prep(P0, W0, W1, W2, W3, K20_39);

      F2(A, B, C, D, E, GET_P_32(P1, 0));
      F2(E, A, B, C, D, GET_P_32(P1, 1));
      F2(D, E, A, B, C, GET_P_32(P1, 2));
      F2(C, D, E, A, B, GET_P_32(P1, 3));
      prep(P1, W1, W2, W3, W0, K20_39);

      F2(B, C, D, E, A, GET_P_32(P2, 0));
      F2(A, B, C, D, E, GET_P_32(P2, 1));
      F2(E, A, B, C, D, GET_P_32(P2, 2));
      F2(D, E, A, B, C, GET_P_32(P2, 3));
      prep(P2, W2, W3, W0, W1, K40_59);

      F2(C, D, E, A, B, GET_P_32(P3, 0));
      F2(B, C, D, E, A, GET_P_32(P3, 1));
      F2(A, B, C, D, E, GET_P_32(P3, 2));
      F2(E, A, B, C, D, GET_P_32(P3, 3));
      prep(P3, W3, W0, W1, W2, K40_59);

      F2(D, E, A, B, C, GET_P_32(P0, 0));
      F2(C, D, E, A, B, GET_P_32(P0, 1));
      F2(B, C, D, E, A, GET_P_32(P0, 2));
      F2(A, B, C, D, E, GET_P_32(P0, 3));
      prep(P0, W0, W1, W2, W3, K40_59);

      F2(E, A, B, C, D, GET_P_32(P1, 0));
      F2(D, E, A, B, C, GET_P_32(P1, 1));
      F2(C, D, E, A, B, GET_P_32(P1, 2));
      F2(B, C, D, E, A, GET_P_32(P1, 3));
      prep(P1, W1, W2, W3, W0, K40_59);

      F3(A, B, C, D, E, GET_P_32(P2, 0));
      F3(E, A, B, C, D, GET_P_32(P2, 1));
      F3(D, E, A, B, C, GET_P_32(P2, 2));
      F3(C, D, E, A, B, GET_P_32(P2, 3));
      prep(P2, W2, W3, W0, W1, K40_59);

      F3(B, C, D, E, A, GET_P_32(P3, 0));
      F3(A, B, C, D, E, GET_P_32(P3, 1));
      F3(E, A, B, C, D, GET_P_32(P3, 2));
      F3(D, E, A, B, C, GET_P_32(P3, 3));
      prep(P3, W3, W0, W1, W2, K60_79);

      F3(C, D, E, A, B, GET_P_32(P0, 0));
      F3(B, C, D, E, A, GET_P_32(P0, 1));
      F3(A, B, C, D, E, GET_P_32(P0, 2));
      F3(E, A, B, C, D, GET_P_32(P0, 3));
      prep(P0, W0, W1, W2, W3, K60_79);

      F3(D, E, A, B, C, GET_P_32(P1, 0));
      F3(C, D, E, A, B, GET_P_32(P1, 1));
      F3(B, C, D, E, A, GET_P_32(P1, 2));
      F3(A, B, C, D, E, GET_P_32(P1, 3));
      prep(P1, W1, W2, W3, W0, K60_79);

      F3(E, A, B, C, D, GET_P_32(P2, 0));
      F3(D, E, A, B, C, GET_P_32(P2, 1));
      F3(C, D, E, A, B, GET_P_32(P2, 2));
      F3(B, C, D, E, A, GET_P_32(P2, 3));
      prep(P2, W2, W3, W0, W1, K60_79);

      F4(A, B, C, D, E, GET_P_32(P3, 0));
      F4(E, A, B, C, D, GET_P_32(P3, 1));
      F4(D, E, A, B, C, GET_P_32(P3, 2));
      F4(C, D, E, A, B, GET_P_32(P3, 3));
      prep(P3, W3, W0, W1, W2, K60_79);

      F4(B, C, D, E, A, GET_P_32(P0, 0));
      F4(A, B, C, D, E, GET_P_32(P0, 1));
      F4(E, A, B, C, D, GET_P_32(P0, 2));
      F4(D, E, A, B, C, GET_P_32(P0, 3));

      F4(C, D, E, A, B, GET_P_32(P1, 0));
      F4(B, C, D, E, A, GET_P_32(P1, 1));
      F4(A, B, C, D, E, GET_P_32(P1, 2));
      F4(E, A, B, C, D, GET_P_32(P1, 3));

      F4(D, E, A, B, C, GET_P_32(P2, 0));
      F4(C, D, E, A, B, GET_P_32(P2, 1));
      F4(B, C, D, E, A, GET_P_32(P2, 2));
      F4(A, B, C, D, E, GET_P_32(P2, 3));

      F4(E, A, B, C, D, GET_P_32(P3, 0));
      F4(D, E, A, B, C, GET_P_32(P3, 1));
      F4(C, D, E, A, B, GET_P_32(P3, 2));
      F4(B, C, D, E, A, GET_P_32(P3, 3));

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);

      input += (hash_block_size() / 16);
      }

#undef GET_P_32
   }

#undef prep00_15
#undef prep

}
/*
* XTEA in SIMD
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(SIMD_32::enabled(), XTEA_SIMD, "XTEA", "simd32", 64);

namespace {

void xtea_encrypt_8(const byte in[64], byte out[64], const u32bit EK[64])
   {
   SIMD_32 L0 = SIMD_32::load_be(in     );
   SIMD_32 R0 = SIMD_32::load_be(in + 16);
   SIMD_32 L1 = SIMD_32::load_be(in + 32);
   SIMD_32 R1 = SIMD_32::load_be(in + 48);

   SIMD_32::transpose(L0, R0, L1, R1);

   for(size_t i = 0; i != 32; i += 2)
      {
      SIMD_32 K0(EK[2*i  ]);
      SIMD_32 K1(EK[2*i+1]);
      SIMD_32 K2(EK[2*i+2]);
      SIMD_32 K3(EK[2*i+3]);

      L0 += (((R0 << 4) ^ (R0 >> 5)) + R0) ^ K0;
      L1 += (((R1 << 4) ^ (R1 >> 5)) + R1) ^ K0;

      R0 += (((L0 << 4) ^ (L0 >> 5)) + L0) ^ K1;
      R1 += (((L1 << 4) ^ (L1 >> 5)) + L1) ^ K1;

      L0 += (((R0 << 4) ^ (R0 >> 5)) + R0) ^ K2;
      L1 += (((R1 << 4) ^ (R1 >> 5)) + R1) ^ K2;

      R0 += (((L0 << 4) ^ (L0 >> 5)) + L0) ^ K3;
      R1 += (((L1 << 4) ^ (L1 >> 5)) + L1) ^ K3;
      }

   SIMD_32::transpose(L0, R0, L1, R1);

   L0.store_be(out);
   R0.store_be(out + 16);
   L1.store_be(out + 32);
   R1.store_be(out + 48);
   }

void xtea_decrypt_8(const byte in[64], byte out[64], const u32bit EK[64])
   {
   SIMD_32 L0 = SIMD_32::load_be(in     );
   SIMD_32 R0 = SIMD_32::load_be(in + 16);
   SIMD_32 L1 = SIMD_32::load_be(in + 32);
   SIMD_32 R1 = SIMD_32::load_be(in + 48);

   SIMD_32::transpose(L0, R0, L1, R1);

   for(size_t i = 0; i != 32; i += 2)
      {
      SIMD_32 K0(EK[63 - 2*i]);
      SIMD_32 K1(EK[62 - 2*i]);
      SIMD_32 K2(EK[61 - 2*i]);
      SIMD_32 K3(EK[60 - 2*i]);

      R0 -= (((L0 << 4) ^ (L0 >> 5)) + L0) ^ K0;
      R1 -= (((L1 << 4) ^ (L1 >> 5)) + L1) ^ K0;

      L0 -= (((R0 << 4) ^ (R0 >> 5)) + R0) ^ K1;
      L1 -= (((R1 << 4) ^ (R1 >> 5)) + R1) ^ K1;

      R0 -= (((L0 << 4) ^ (L0 >> 5)) + L0) ^ K2;
      R1 -= (((L1 << 4) ^ (L1 >> 5)) + L1) ^ K2;

      L0 -= (((R0 << 4) ^ (R0 >> 5)) + R0) ^ K3;
      L1 -= (((R1 << 4) ^ (R1 >> 5)) + R1) ^ K3;
      }

   SIMD_32::transpose(L0, R0, L1, R1);

   L0.store_be(out);
   R0.store_be(out + 16);
   L1.store_be(out + 32);
   R1.store_be(out + 48);
   }

}

/*
* XTEA Encryption
*/
void XTEA_SIMD::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_EK()[0]);

   while(blocks >= 8)
      {
      xtea_encrypt_8(in, out, KS);
      in += 8 * BLOCK_SIZE;
      out += 8 * BLOCK_SIZE;
      blocks -= 8;
      }

   if(blocks)
     XTEA::encrypt_n(in, out, blocks);
   }

/*
* XTEA Decryption
*/
void XTEA_SIMD::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_EK()[0]);

   while(blocks >= 8)
      {
      xtea_decrypt_8(in, out, KS);
      in += 8 * BLOCK_SIZE;
      out += 8 * BLOCK_SIZE;
      blocks -= 8;
      }

   if(blocks)
     XTEA::decrypt_n(in, out, blocks);
   }

}

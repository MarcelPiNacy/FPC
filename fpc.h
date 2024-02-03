/*
  Copyright (c) 2024 Marcel Pi Nacy

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/*
    Martin Burtscher and Paruj Ratanaworabhan's paper:
    https://www.researchgate.net/publication/224323445_FPC_A_High-Speed_Compressor_for_Double-Precision_Floating-Point_Data
*/

#ifndef FPC_INCLUDED
#define FPC_INCLUDED

#include <stdint.h>
#include <stddef.h>

#ifndef FPC_CALL
  #define FPC_CALL
#endif

#ifndef FPC_ATTR
  #define FPC_ATTR
#endif

#if defined(__clang__) || defined(__GNUC__)
  #define FPC_RESTRICT __restrict__
#elif defined(_MSC_VER)
  #define FPC_RESTRICT __restrict
#else
  #define FPC_RESTRICT
#endif

#define FPC_LEAST_FREQUENT_LZBC 4
#define FPC_UPPER_BOUND_METADATA(COUNT) ((size_t)((COUNT) + 1) / 2)
#define FPC_UPPER_BOUND_DATA(COUNT) ((size_t)(COUNT) * 8)
#define FPC_UPPER_BOUND(COUNT) FPC_UPPER_BOUND_METADATA((COUNT)) + FPC_UPPER_BOUND_DATA((COUNT))
#define FPC_DEFAULT_HASH_ARGS { 6, 48, 2, 40 }

#define FPC32_UPPER_BOUND_METADATA(COUNT) ((size_t)((COUNT) + 1) / 2)
#define FPC32_UPPER_BOUND_DATA(COUNT) ((size_t)(COUNT) * 4)
#define FPC32_UPPER_BOUND(COUNT) FPC32_UPPER_BOUND_METADATA((COUNT)) + FPC32_UPPER_BOUND_DATA((COUNT))
#define FPC32_DEFAULT_HASH_ARGS { 1, 22, 4, 23 }

typedef struct fpc_hash_args_t
{
  uint8_t fcm_lshift;
  uint8_t fcm_rshift;
  uint8_t dfcm_lshift;
  uint8_t dfcm_rshift;
} fpc_hash_args_t;

typedef struct fpc_context_t
{
  uint64_t* fcm;
  uint64_t* dfcm;
  // The size, in elements, of the array pointed to by "fcm".
  size_t fcm_size;
  // The size, in elements, of the array pointed to by "dfcm".
  size_t dfcm_size;
  // Seed value.
  double seed;
  // Custom options for the FCM and DFCM hash functions.
  fpc_hash_args_t hash_args;
} fpc_context_t;

typedef fpc_context_t* FPC_RESTRICT fpc_context_ptr_t;

typedef struct fpc32_context_t
{
  uint32_t* fcm;
  uint32_t* dfcm;
  // The size, in elements, of the array pointed to by "fcm".
  uint32_t fcm_size;
  // The size, in elements, of the array pointed to by "dfcm".
  uint32_t dfcm_size;
  // Seed value.
  float seed;
  // Custom options for the FCM and DFCM hash functions.
  fpc_hash_args_t hash_args;
} fpc32_context_t;

typedef fpc32_context_t* FPC_RESTRICT fpc32_context_ptr_t;

FPC_ATTR size_t FPC_CALL fpc_encode_separate(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* out_headers,
  void* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode_separate(
  fpc_context_ptr_t ctx,
  const void* headers,
  const void* compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR size_t FPC_CALL fpc_encode(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode(
  fpc_context_ptr_t ctx,
  const void* compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR size_t FPC_CALL fpc32_encode_separate(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* out_headers,
  void* out_compressed);

FPC_ATTR void FPC_CALL fpc32_decode_separate(
  fpc32_context_ptr_t ctx,
  const void* headers,
  const void* compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR size_t FPC_CALL fpc32_encode(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* out_compressed);

FPC_ATTR void FPC_CALL fpc32_decode(
  fpc32_context_ptr_t ctx,
  const void* compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count);

#endif



#define FPC_IMPLEMENTATION
#ifdef FPC_IMPLEMENTATION

#include <string.h>

#if __has_include(<stdbool.h>)
  #include <stdbool.h>
#endif

#if defined(_DEBUG) || !defined(NDEBUG)
    #define FPC_DEBUG
#endif

#if defined(__clang__) || defined(__GNUC__)
  #ifdef __has_builtin
    #if __has_builtin(__builtin_clz)
      #define FPC_CLZ32 (uint_fast8_t)__builtin_clz
    #endif
    #if __has_builtin(__builtin_clzll)
      #define FPC_CLZ64 (uint_fast8_t)__builtin_clzll
    #endif
    #if __has_builtin(__builtin_ctz)
      #define FPC_CTZ32 (uint_fast8_t)__builtin_ctz
    #endif
    #if __has_builtin(__builtin_ctzll)
      #define FPC_CTZ64 (uint_fast8_t)__builtin_ctzll
    #endif
    #if __has_builtin(__builtin_bswap32)
      #define FPC_BSWAP32 __builtin_bswap32
    #endif
    #if __has_builtin(__builtin_bswap64)
      #define FPC_BSWAP64 __builtin_bswap64
    #endif
    #if __has_builtin(__builtin_nontemporal_load)
      #define FPC_LOAD_NT(P) __builtin_nontemporal_load(P)
      #define FPC_LOAD_NT_U32(P) FPC_LOAD_NT((const uint32_t*)(P))
      #define FPC_LOAD_NT_U64(P) FPC_LOAD_NT((const uint64_t*)(P))
    #endif
    #if __has_builtin(__builtin_nontemporal_load)
      #define FPC_STORE_NT(P, V) __builtin_nontemporal_store(V, P)
      #define FPC_STORE_NT_U32(P, V) FPC_STORE_NT((const uint32_t*)(P), (V))
      #define FPC_STORE_NT_U64(P, V) FPC_STORE_NT((const uint64_t*)(P), (V))
    #endif
    #if __has_builtin(__builtin_memcpy)
      #define FPC_MEMCPY (void)__builtin_memcpy
    #endif
    #if __has_builtin(__builtin_memcpy_inline)
      #define FPC_MEMCPY_FIXED (void)__builtin_memcpy_inline
    #endif
    #if __has_builtin(__builtin_memset)
      #define FPC_MEMSET (void)__builtin_memset
    #endif
    #if __has_builtin(__builtin_memset_inline)
      #define FPC_MEMSET_FIXED (void)__builtin_memset_inline
    #endif
    #if __has_builtin(__builtin_assume)
      #define FPC_ASSUME __builtin_assume
    #elif defined(__has_attribute)
        #if __has_attribute(__assume__)
            #define FPC_ASSUME(E) __attribute__((__assume__(E)))
        #endif
    #endif
  #endif
  #ifdef __has_attribute
    #if __has_attribute(aligned)
      #define FPC_ALIGN(N) __attribute__((aligned(N)))
    #endif
  #endif
  #define FPC_RESTRICT __restrict__

  #pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#elif defined(_MSC_VER)
  #if __has_include(<intrin.h>)
    #include <intrin.h>
  #endif
  #define FPC_CLZ32 (uint_fast8_t)__lzcnt
  #define FPC_CLZ64 (uint_fast8_t)__lzcnt64
  #define FPC_CTZ32 (uint_fast8_t)_tzcnt_u32
  #define FPC_CTZ64 (uint_fast8_t)_tzcnt_u64
  #define FPC_ALIGN(N) __declspec(align(N))
  #define FPC_RESTRICT __restrict
  #define FPC_ASSUME _assume
#endif

#ifdef FPC_DEBUG
  #include <assert.h>
  #define FPC_INVARIANT assert
#elif defined(FPC_ASSUME)
  #define FPC_INVARIANT FPC_ASSUME
#else
  #define FPC_INVARIANT(unused)
#endif

#define FPC_FCM_HASH_UPDATE(HASH, VALUE)   HASH = ((HASH << ctx->hash_args.fcm_lshift) ^ (size_t)((VALUE) >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask
#define FPC_DFCM_HASH_UPDATE(HASH, VALUE)  HASH = ((HASH << ctx->hash_args.dfcm_lshift) ^ (size_t)((VALUE) >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask

#define FPC_IS_ALIGNED(PTR, ALIGN) (((size_t)(PTR) & (size_t)((ALIGN) - 1)) == 0)

#ifndef FPC_MEMCPY
  #define FPC_MEMCPY (void)memcpy
#endif

#ifndef FPC_MEMSET
  #define FPC_MEMSET (void)memset
#endif

#ifndef FPC_LOAD_NT_U32
  #define FPC_LOAD_NT_U32(P) (*(const uint32_t*)(P))
#endif

#ifndef FPC_LOAD_NT_U64
  #define FPC_LOAD_NT_U64(P) (*(const uint64_t*)(P))
#endif

#ifndef FPC_STORE_NT_U32
  #define FPC_STORE_NT_U32(P, V) (*(uint32_t*)(P)) = (V)
#endif

#ifndef FPC_STORE_NT_U64
  #define FPC_STORE_NT_U64(P, V) (*(uint64_t*)(P)) = (V)
#endif

#ifndef FPC_MEMCPY_FIXED
  #define FPC_MEMCPY_FIXED (void)memcpy
#endif

#ifndef FPC_MEMSET_FIXED
  #define FPC_MEMSET_FIXED (void)memset
#endif

#ifdef FPC_STATIC_BYTE_ORDER

  #define FPC_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  #define FPC_BIG_ENDIAN (!FPC_LITTLE_ENDIAN)

#else

static bool fpc_is_little_endian()
{
  uint8_t x[2];
  x[0] = 1;
  x[1] = 0;
  return 1 == *(const uint16_t*)x;
}

  #define FPC_LITTLE_ENDIAN fpc_is_little_endian()
  #define FPC_BIG_ENDIAN (!FPC_LITTLE_ENDIAN)

#endif



#define FPC_FCM_UPDATE(value) \
  ctx->fcm[fcm_hash] = value; \
  FPC_FCM_HASH_UPDATE(fcm_hash, value); \
  predicted = ctx->fcm[fcm_hash]

#define FPC_DFCM_UPDATE(value) \
  ctx->dfcm[dfcm_hash] = delta; \
  FPC_DFCM_HASH_UPDATE(dfcm_hash, delta); \
  predicted_delta = ctx->dfcm[dfcm_hash]

#define FPC_ENCODE_STEP(value) \
  fcm_xor = value ^ predicted; \
  FPC_FCM_UPDATE(value); \
  delta = value - prior_value; \
  dfcm_xor = value ^ (prior_value + predicted_delta); \
  FPC_DFCM_UPDATE(value); \
  prior_value = value

#define FPC_DECODE_STEP(value, type) \
  value ^= type ? (prior_value + predicted_delta) : predicted; \
  prior_value = value; \
  FPC_FCM_UPDATE(value); \
  delta = value - prior_value; \
  FPC_DFCM_UPDATE(value)

#define FPC_DECODE_VALUE(value, type, size) \
  FPC_MEMCPY(&value, in, size); \
  in += size; \
  FPC_DECODE_STEP(value, type)

FPC_ATTR size_t FPC_CALL fpc_encode_separate(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed)
{
  const double* FPC_RESTRICT end;
  uint8_t* FPC_RESTRICT compressed_begin;
  uint8_t* FPC_RESTRICT out;
  uint8_t* FPC_RESTRICT out_h;

  uint64_t
    value,
    fcm_mod_mask,
    dfcm_mod_mask,
    fcm_hash,
    dfcm_hash,
    predicted,
    predicted_delta,
    delta,
    prior_value,
    fcm_xor,
    dfcm_xor;

  uint_fast8_t
    type,
    lzbc,
    header,
    i;

  FPC_INVARIANT((uint8_t* FPC_RESTRICT)out_headers != (uint8_t* FPC_RESTRICT)out_compressed);
  FPC_INVARIANT(
    (uint8_t* FPC_RESTRICT)out_headers < (uint8_t* FPC_RESTRICT)out_compressed ||
    (uint8_t* FPC_RESTRICT)out_headers + value_count >= (uint8_t* FPC_RESTRICT)out_compressed);

  end = values + value_count;
  if (values == end)
    return 0;
  out_h = (uint8_t * FPC_RESTRICT)out_headers;
  compressed_begin = out = (uint8_t* FPC_RESTRICT)out_compressed;
  prior_value = *(const uint64_t*)&ctx->seed;
  fcm_hash = dfcm_hash = predicted = predicted_delta = 0;
  fcm_mod_mask = ctx->fcm_size - 1;
  dfcm_mod_mask = ctx->dfcm_size - 1;
  while (values != end)
  {
    header = 0;
    #ifdef __clang__
      #pragma clang unroll(full)
    #endif
    for (i = 0; i != 2 && values != end; ++i)
    {
      value = FPC_LOAD_NT_U64(values);
      ++values;

      fcm_xor = value ^ predicted;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      predicted = ctx->fcm[fcm_hash];
      delta = value - prior_value;
      dfcm_xor = value ^ (prior_value + predicted_delta);
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      predicted_delta = ctx->dfcm[dfcm_hash];
      prior_value = value;

      type = fcm_xor > dfcm_xor;
      value = type ? dfcm_xor : fcm_xor;
      lzbc = FPC_CLZ64(value) >> 3;
      lzbc -= (lzbc == FPC_LEAST_FREQUENT_LZBC);
      header = ((type << 3) | lzbc) << (1 << 2);

      lzbc = 8 - lzbc;
      if (FPC_LITTLE_ENDIAN)
        value = FPC_BSWAP64(value);
      FPC_MEMCPY(out, &value, lzbc);
    }
    *out_h = header;
    ++out_h;
  }
  return (size_t)(out - compressed_begin) + (value_count + 1) / 2;
}

FPC_ATTR void FPC_CALL fpc_decode_separate(
  fpc_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count)
{
  const uint8_t* FPC_RESTRICT in;
  const uint8_t* FPC_RESTRICT in_headers;
  double* FPC_RESTRICT end;
  uint_fast64_t
    fcm_mod_mask,
    dfcm_mod_mask,
    fcm_hash,
    dfcm_hash,
    predicted,
    predicted_delta,
    delta,
    prior_value,
    value;
  uint_fast8_t
    type,
    lzbc,
    header,
    i;

  in = (const uint8_t* FPC_RESTRICT)compressed;
  in_headers = (const uint8_t* FPC_RESTRICT)headers;
  end = out_values + out_count;
  fcm_mod_mask = ctx->fcm_size - 1;
  dfcm_mod_mask = ctx->dfcm_size - 1;
  prior_value = *(const uint64_t*)&ctx->seed;
  fcm_hash = dfcm_hash = predicted = delta = predicted_delta = 0;
  while (out_values != end)
  {
    header = *in_headers++;
    for (i = 0; i != 2 && out_values != end; ++i)
    {
      type = header & 8;
      lzbc = 8 - (header & 7);

      FPC_MEMCPY(&value, in, lzbc);
      if (FPC_LITTLE_ENDIAN)
        value = FPC_BSWAP64(value);

      value ^= type ? (prior_value + predicted_delta) : predicted;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      predicted = ctx->fcm[fcm_hash];
      delta = value - prior_value;
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      predicted_delta = ctx->dfcm[dfcm_hash];

      FPC_STORE_NT_U64(out_values, value);
      ++out_values;

      header >>= 4;
    }
  }
}

FPC_ATTR size_t FPC_CALL fpc_encode(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_compressed)
{
  return fpc_encode_separate(
    ctx,
    values,
    value_count,
    out_compressed,
    (uint8_t* FPC_RESTRICT)out_compressed + FPC_UPPER_BOUND_METADATA(value_count));
}

FPC_ATTR void FPC_CALL fpc_decode(
  fpc_context_ptr_t ctx,
  const void* FPC_RESTRICT compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count)
{
  fpc_decode_separate(
    ctx,
    compressed,
    (const uint8_t* FPC_RESTRICT)compressed + FPC_UPPER_BOUND_METADATA(out_count),
    out_values,
    out_count);
}

FPC_ATTR size_t FPC_CALL fpc32_encode_separate(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed)
{
#if 0
  const float* FPC_RESTRICT end;
  uint8_t* FPC_RESTRICT out;
  uint8_t* FPC_RESTRICT out_h;
  uint_fast32_t
    type_a,
    type_b,
    lzbc_a,
    lzbc_b,
    fcm_mod_mask,
    dfcm_mod_mask,
    fcm_hash,
    dfcm_hash,
    predicted,
    predicted_delta,
    delta,
    prior_value,
    fcm_xor,
    dfcm_xor;
  FPC_ALIGN(8)
  uint32_t tmp[2];

  FPC_INVARIANT((uint8_t* FPC_RESTRICT)out_headers != (uint8_t* FPC_RESTRICT)out_compressed);
  FPC_INVARIANT(
    (uint8_t* FPC_RESTRICT)out_headers < (uint8_t* FPC_RESTRICT)out_compressed ||
    (uint8_t* FPC_RESTRICT)out_headers + value_count >= (uint8_t* FPC_RESTRICT)out_compressed);
  end = values + value_count;
  out = (uint8_t* FPC_RESTRICT)out_compressed;
  out_h = (uint8_t* FPC_RESTRICT)out_headers;
  prior_value = *(const uint32_t*)&ctx->seed;
  fcm_hash = dfcm_hash = predicted = delta = predicted_delta = 0;
  fcm_mod_mask = ctx->fcm_size - 1;
  dfcm_mod_mask = ctx->dfcm_size - 1;
  while ((end - values) >= 2)
  {
    FPC_MEMCPY_FIXED(tmp, values, 8);
    values += 2;
    FPC_ENCODE_STEP(tmp[0]);
    type_a = (uint_fast32_t)(fcm_xor > dfcm_xor) << 3;
    tmp[0] = type_a ? dfcm_xor : fcm_xor;
    FPC_ENCODE_STEP(tmp[1]);
    type_b = (uint_fast32_t)(fcm_xor > dfcm_xor) << 3;
    tmp[1] = type_b ? dfcm_xor : fcm_xor;
    lzbc_a = FPC_CLZ32(tmp[0]) >> 3;
    lzbc_b = FPC_CLZ32(tmp[1]) >> 3;
    *out_h++ = (uint8_t)(((type_b | lzbc_b) << 4) | (type_a | lzbc_a));
    lzbc_a = 4 - lzbc_a;
    lzbc_b = 4 - lzbc_b;
    FPC_MEMCPY(out, tmp, lzbc_a);
    out += lzbc_a;
    FPC_MEMCPY(out, &tmp[1], lzbc_b);
    out += lzbc_b;
  }
  if (values != end)
  {
    FPC_MEMCPY_FIXED(tmp, values, 8);
    FPC_ENCODE_STEP(tmp[0]);
    type_a = fcm_xor > dfcm_xor;
    tmp[0] = type_a ? dfcm_xor : fcm_xor;
    type_a <<= 3;
    lzbc_a = FPC_CLZ32(tmp[0]) >> 3;
    lzbc_a -= (lzbc_a == FPC_LEAST_FREQUENT_LZBC);
    *out_h =
      (uint8_t)(type_a | (lzbc_a - (lzbc_a > FPC_LEAST_FREQUENT_LZBC)));
    ++out_h;
    FPC_MEMCPY(out, tmp, lzbc_a);
    out += lzbc_a;
  }
  return (size_t)(out - (uint8_t* FPC_RESTRICT)out_compressed) + (value_count + 1) / 2;
#else
  return 0;
#endif
}

FPC_ATTR void FPC_CALL fpc32_decode_separate(
  fpc32_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count)
{
#if 0
  const uint8_t* FPC_RESTRICT in;
  const uint8_t* FPC_RESTRICT in_headers;
  float* FPC_RESTRICT end;
  uint_fast32_t
    lzbc,
    header,
    fcm_mod_mask,
    dfcm_mod_mask,
    fcm_hash,
    dfcm_hash,
    predicted,
    predicted_delta,
    delta,
    prior_value;
  FPC_ALIGN(8)
  uint32_t tmp[2];

  in = (const uint8_t* FPC_RESTRICT)compressed;
  in_headers = (const uint8_t* FPC_RESTRICT)headers;
  end = out_values + out_count;
  fcm_mod_mask = ctx->fcm_size - 1;
  dfcm_mod_mask = ctx->dfcm_size - 1;
  FPC_MEMCPY_FIXED(&prior_value, &ctx->seed, 4);
  fcm_hash = dfcm_hash = predicted = delta = predicted_delta = 0;
  prior_value = *(const uint32_t*)&ctx->seed;
  while (out_values != end)
  {
    FPC_MEMSET_FIXED(tmp, 0, 8);
    header = *in_headers++;
    FPC_DECODE_VALUE(tmp[0]);
    ++out_values;
    if (out_values == end)
    {
      FPC_MEMCPY_FIXED(out_values, tmp, 4);
      return;
    }
    header >>= 4;
    FPC_DECODE_VALUE(tmp[1]);
    FPC_MEMCPY_FIXED(out_values - 1, tmp, 8);
    ++out_values;
  }
#endif
}

FPC_ATTR size_t FPC_CALL fpc32_encode(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_compressed)
{
  return fpc32_encode_separate(
    ctx,
    values,
    value_count,
    out_compressed,
    (uint8_t* FPC_RESTRICT)out_compressed + FPC32_UPPER_BOUND_METADATA(value_count));
}

FPC_ATTR void FPC_CALL fpc32_decode(
  fpc32_context_ptr_t ctx,
  const void* FPC_RESTRICT compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count)
{
  fpc32_decode_separate(
    ctx,
    compressed,
    (const uint8_t* FPC_RESTRICT)compressed + FPC32_UPPER_BOUND_METADATA(out_count),
    out_values,
    out_count);
}

#endif

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
  uint64_t* FPC_RESTRICT fcm;
  uint64_t* FPC_RESTRICT dfcm;
  // The size, in elements, of the array pointed to by "fcm".
  size_t fcm_size;
  // The size, in elements, of the array pointed to by "dfcm".
  size_t dfcm_size;
  // Seed value.
  double delta_seed;
  // Custom options for the FCM and DFCM hash functions.
  fpc_hash_args_t hash_args;
} fpc_context_t;

typedef fpc_context_t* FPC_RESTRICT fpc_context_ptr_t;

typedef struct fpc32_context_t
{
  uint32_t* FPC_RESTRICT fcm;
  uint32_t* FPC_RESTRICT dfcm;
  // The size, in elements, of the array pointed to by "fcm".
  size_t fcm_size;
  // The size, in elements, of the array pointed to by "dfcm".
  size_t dfcm_size;
  // Seed value.
  float delta_seed;
  // Custom options for the FCM and DFCM hash functions.
  fpc_hash_args_t hash_args;
} fpc32_context_t;

typedef fpc32_context_t* FPC_RESTRICT fpc32_context_ptr_t;

FPC_ATTR void FPC_CALL fpc_context_init(
  fpc_context_ptr_t ctx,
  uint64_t* FPC_RESTRICT fcm,
  uint64_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size,
  fpc_hash_args_t hash_args,
  double delta_seed);

FPC_ATTR void FPC_CALL fpc_context_init_default(
  fpc_context_ptr_t ctx,
  uint64_t* FPC_RESTRICT fcm,
  uint64_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size);

FPC_ATTR void FPC_CALL fpc_context_reset(
  fpc_context_ptr_t ctx);

FPC_ATTR size_t FPC_CALL fpc_encode_size(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count);

FPC_ATTR size_t FPC_CALL fpc_encode(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_compressed);

FPC_ATTR size_t FPC_CALL fpc_encode_separate(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed);

FPC_ATTR void FPC_CALL fpc_decode(
  fpc_context_ptr_t ctx,
  const void* FPC_RESTRICT compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR void FPC_CALL fpc_decode_separate(
  fpc_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR void FPC_CALL fpc32_context_init(
  fpc32_context_ptr_t ctx,
  uint32_t* FPC_RESTRICT fcm,
  uint32_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size,
  fpc_hash_args_t hash_args,
  float delta_seed);

FPC_ATTR void FPC_CALL fpc32_context_init_default(
  fpc32_context_ptr_t ctx,
  uint32_t* FPC_RESTRICT fcm,
  uint32_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size);

FPC_ATTR void FPC_CALL fpc32_context_reset(
  fpc32_context_ptr_t ctx);

FPC_ATTR size_t FPC_CALL fpc32_encode_size(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count);

FPC_ATTR size_t FPC_CALL fpc32_encode_separate(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed);

FPC_ATTR size_t FPC_CALL fpc32_encode(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_compressed);

FPC_ATTR void FPC_CALL fpc32_decode_separate(
  fpc32_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count);

FPC_ATTR void FPC_CALL fpc32_decode(
  fpc32_context_ptr_t ctx,
  const void* FPC_RESTRICT compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count);

#endif



#ifdef FPC_IMPLEMENTATION

#if __has_include(<stdbool.h>)
  #include <stdbool.h>
  #define FPC_BOOL bool
#else
  #ifdef __cplusplus
    #define FPC_BOOL bool
  #else
    #define FPC_BOOL int
  #endif
#endif

#if defined(__clang__) || defined(__GNUC__)
  #ifdef __has_builtin
    #if __has_builtin(__builtin_expect)
      #define FPC_LIKELY_IF(C) if (__builtin_expect((long)(C), 1))
      #define FPC_UNLIKELY_IF(C) if (__builtin_expect((long)(C), 0))
    #endif
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
      #define FPC_LOAD_NT_U32(P) FPC_LOAD_NT((const uint32_t* FPC_RESTRICT)(P))
      #define FPC_LOAD_NT_U64(P) FPC_LOAD_NT((const uint64_t* FPC_RESTRICT)(P))
    #endif
    #if __has_builtin(__builtin_nontemporal_load)
      #define FPC_STORE_NT(P, V) __builtin_nontemporal_store(V, P)
      #define FPC_STORE_NT_U32(P, V) FPC_STORE_NT((const uint32_t* FPC_RESTRICT)(P), (V))
      #define FPC_STORE_NT_U64(P, V) FPC_STORE_NT((const uint64_t* FPC_RESTRICT)(P), (V))
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
#elif defined(_MSC_VER)
  #if __has_include(<intrin.h>)
    #include <intrin.h>
  #endif
  #define FPC_CLZ32 (uint_fast8_t)__lzcnt
  #define FPC_CLZ64 (uint_fast8_t)__lzcnt64
  #define FPC_CTZ32 (uint_fast8_t)_tzcnt_u32
  #define FPC_CTZ64 (uint_fast8_t)_tzcnt_u64
  #define FPC_BSWAP32 (uint32_t)_byteswap_ulong
  #define FPC_BSWAP64 (uint64_t)_byteswap_uint64
  #define FPC_ALIGN(N) __declspec(align(N))
  #define FPC_RESTRICT __restrict
  #define FPC_ASSUME __assume
#endif

#ifdef FPC_DEBUG
  #include <assert.h>
  #define FPC_INVARIANT assert
#elif defined(FPC_ASSUME)
  #define FPC_INVARIANT FPC_ASSUME
#else
  #define FPC_INVARIANT(unused)
#endif

#define FPC_FCM_HASH_UPDATE(HASH, VALUE) \
  HASH = ((HASH << ctx->hash_args.fcm_lshift) ^ \
  (size_t)((VALUE) >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask

#define FPC_DFCM_HASH_UPDATE(HASH, VALUE) \
  HASH = ((HASH << ctx->hash_args.dfcm_lshift) ^ \
  (size_t)((VALUE) >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask

#define FPC_IS_ALIGNED(PTR, ALIGN) \
  (((size_t)(PTR) & (size_t)((ALIGN) - 1)) == 0)

#if !defined(FPC_MEMCPY) || !defined(FPC_MEMSET)
  #include <string.h>
#endif

#ifndef FPC_MEMCPY
  #define FPC_MEMCPY (void)memcpy
#endif

#ifndef FPC_MEMSET
  #define FPC_MEMSET (void)memset
#endif

#ifndef FPC_MEMCPY_FIXED
  #define FPC_MEMCPY_FIXED FPC_MEMCPY
#endif

#ifndef FPC_MEMSET_FIXED
  #define FPC_MEMSET_FIXED FPC_MEMSET
#endif

#ifndef FPC_LOAD_NT_U32
  #define FPC_LOAD_NT_U32(P) (*(const uint32_t* FPC_RESTRICT)(P))
#endif

#ifndef FPC_LOAD_NT_U64
  #define FPC_LOAD_NT_U64(P) (*(const uint64_t* FPC_RESTRICT)(P))
#endif

#ifndef FPC_STORE_NT_U32
  #define FPC_STORE_NT_U32(P, V) (*(uint32_t* FPC_RESTRICT)(P)) = (V)
#endif

#ifndef FPC_STORE_NT_U64
  #define FPC_STORE_NT_U64(P, V) (*(uint64_t* FPC_RESTRICT)(P)) = (V)
#endif

#define FPC_IS_POW2(x) (((x) != 0) && (((x) & ((x) - 1)) != 0))

FPC_ATTR void FPC_CALL fpc_context_init(
  fpc_context_ptr_t ctx,
  uint64_t* FPC_RESTRICT fcm,
  uint64_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size,
  fpc_hash_args_t hash_args,
  double delta_seed)
{
  FPC_INVARIANT(FPC_IS_POW2(fcm_size));
  FPC_INVARIANT(FPC_IS_POW2(dfcm_size));
  ctx->fcm = fcm;
  ctx->dfcm = dfcm;
  ctx->fcm_size = fcm_size;
  ctx->dfcm_size = dfcm_size;
  ctx->hash_args = hash_args;
  ctx->delta_seed = delta_seed;
}

FPC_ATTR void FPC_CALL fpc_context_init_default(
  fpc_context_ptr_t ctx,
  uint64_t* FPC_RESTRICT fcm,
  uint64_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size)
{
  const fpc_hash_args_t hash_args = FPC_DEFAULT_HASH_ARGS;
  fpc_context_init(ctx, fcm, dfcm, fcm_size, dfcm_size, hash_args, 0.0);
}

FPC_ATTR void FPC_CALL fpc_context_reset(
  fpc_context_ptr_t ctx)
{
  FPC_MEMSET(ctx->fcm, 0, ctx->fcm_size * sizeof(uint64_t));
  FPC_MEMSET(ctx->dfcm, 0, ctx->dfcm_size * sizeof(uint64_t));
}

FPC_ATTR size_t FPC_CALL fpc_encode_size(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count)
{
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const double* FPC_RESTRICT const end = values + value_count;
  size_t size;
  uint64_t
    value, value_xor,
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    fcm_xor, dfcm_xor;
  uint_fast8_t type, lzbc, i;
  size = 0;
  if (values == end)
    return size;
  prior_value = *(const uint64_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      value = FPC_LOAD_NT_U64(values);
      ++values;
      fcm_xor = value ^ fcm_prediction;
      dfcm_xor = value ^ dfcm_prediction;
      type = fcm_xor > dfcm_xor;
      value_xor = type ? dfcm_xor : fcm_xor;
      lzbc = FPC_CLZ64(value_xor) >> 3;
      lzbc -= (lzbc == FPC_LEAST_FREQUENT_LZBC);
      lzbc = 8 - lzbc;
      FPC_INVARIANT(lzbc <= 8);
      size += lzbc;
      FPC_UNLIKELY_IF (values == end)
        break;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
    ++size;
  } while (values != end);
  return size;
}

FPC_ATTR size_t FPC_CALL fpc_encode_separate(
  fpc_context_ptr_t ctx,
  const double* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed)
{
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const double* FPC_RESTRICT const end = values + value_count;
  uint8_t* FPC_RESTRICT compressed_begin;
  uint8_t* FPC_RESTRICT out;
  uint8_t* FPC_RESTRICT out_h;
  uint64_t
    value, value_xor,
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    fcm_xor, dfcm_xor;
  uint_fast8_t
    type, lzbc,
    header, i;
  FPC_INVARIANT((uint8_t* FPC_RESTRICT)out_headers != (uint8_t* FPC_RESTRICT)out_compressed);
  FPC_INVARIANT(
    (uint8_t* FPC_RESTRICT)out_headers < (uint8_t* FPC_RESTRICT)out_compressed ||
    (uint8_t* FPC_RESTRICT)out_headers + value_count >= (uint8_t* FPC_RESTRICT)out_compressed);
  if (values == end)
    return 0;
  out_h = (uint8_t * FPC_RESTRICT)out_headers;
  compressed_begin = out = (uint8_t* FPC_RESTRICT)out_compressed;
  prior_value = *(const uint64_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    header = 0;
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      value = FPC_LOAD_NT_U64(values);
      ++values;
      fcm_xor = value ^ fcm_prediction;
      dfcm_xor = value ^ dfcm_prediction;
      type = fcm_xor > dfcm_xor;
      value_xor = type ? dfcm_xor : fcm_xor;
      lzbc = FPC_CLZ64(value_xor) >> 3;
      lzbc -= (lzbc == FPC_LEAST_FREQUENT_LZBC);
      header |= (((type << 3) | lzbc)) << (i << 2);
      lzbc = 8 - lzbc;
      FPC_INVARIANT(lzbc <= 8);
      FPC_MEMCPY(out, &value_xor, lzbc);
      out += lzbc;
      FPC_UNLIKELY_IF (values == end)
        break;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
    *out_h = header;
    ++out_h;
  } while (values != end);
  return (size_t)(out - compressed_begin) + (value_count + 1) / 2;
}

FPC_ATTR void FPC_CALL fpc_decode_separate(
  fpc_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  double* FPC_RESTRICT out_values,
  size_t out_count)
{
  double* FPC_RESTRICT const end = out_values + out_count;
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const uint8_t* FPC_RESTRICT in;
  const uint8_t* FPC_RESTRICT in_headers;
  uint64_t
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    value;
  uint_fast8_t
    type, lzbc,
    header, i;
  if (out_values == end)
    return;
  in = (const uint8_t* FPC_RESTRICT)compressed;
  in_headers = (const uint8_t* FPC_RESTRICT)headers;
  prior_value = *(const uint64_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    header = *in_headers;
    ++in_headers;
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      type = header & 8;
      lzbc = (header & 7);
      lzbc = 8 - lzbc;
      header >>= 4;
      value = 0;
      FPC_MEMCPY(&value, in, lzbc);
      value ^= type ? dfcm_prediction : fcm_prediction;
      FPC_STORE_NT_U64(out_values, value);
      ++out_values;
      FPC_UNLIKELY_IF (out_values == end)
        return;
      in += lzbc;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
  } while (out_values != end);
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

FPC_ATTR void FPC_CALL fpc32_context_init(
  fpc32_context_ptr_t ctx,
  uint32_t* FPC_RESTRICT fcm,
  uint32_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size,
  fpc_hash_args_t hash_args,
  float delta_seed)
{
  FPC_INVARIANT(FPC_IS_POW2(fcm_size));
  FPC_INVARIANT(FPC_IS_POW2(dfcm_size));
  ctx->fcm = fcm;
  ctx->dfcm = dfcm;
  ctx->fcm_size = fcm_size;
  ctx->dfcm_size = dfcm_size;
  ctx->hash_args = hash_args;
  ctx->delta_seed = delta_seed;
}

FPC_ATTR void FPC_CALL fpc32_context_init_default(
  fpc32_context_ptr_t ctx,
  uint32_t* FPC_RESTRICT fcm,
  uint32_t* FPC_RESTRICT dfcm,
  size_t fcm_size,
  size_t dfcm_size)
{
  const fpc_hash_args_t hash_args = FPC32_DEFAULT_HASH_ARGS;
  fpc32_context_init(ctx, fcm, dfcm, fcm_size, dfcm_size, hash_args, 0.0F);
}

FPC_ATTR void FPC_CALL fpc32_context_reset(
  fpc32_context_ptr_t ctx)
{
  FPC_MEMSET(ctx->fcm, 0, ctx->fcm_size * sizeof(uint32_t));
  FPC_MEMSET(ctx->dfcm, 0, ctx->dfcm_size * sizeof(uint32_t));
}

FPC_ATTR size_t FPC_CALL fpc32_encode_size(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count)
{
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const float* FPC_RESTRICT const end = values + value_count;
  size_t size;
  uint32_t
    value, value_xor,
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    fcm_xor, dfcm_xor;
  uint_fast8_t type, lzbc, i;
  size = 0;
  if (values == end)
    return size;
  prior_value = *(const uint32_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      value = FPC_LOAD_NT_U64(values);
      ++values;
      fcm_xor = value ^ fcm_prediction;
      dfcm_xor = value ^ dfcm_prediction;
      type = fcm_xor > dfcm_xor;
      value_xor = type ? dfcm_xor : fcm_xor;
      lzbc = FPC_CLZ32(value_xor) >> 3;
      lzbc -= (lzbc == FPC_LEAST_FREQUENT_LZBC);
      lzbc = 4 - lzbc;
      FPC_INVARIANT(lzbc <= 4);
      size += lzbc;
      FPC_UNLIKELY_IF (values == end)
        break;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
    ++size;
  } while (values != end);
  return size;
}

FPC_ATTR size_t FPC_CALL fpc32_encode_separate(
  fpc32_context_ptr_t ctx,
  const float* FPC_RESTRICT values,
  size_t value_count,
  void* FPC_RESTRICT out_headers,
  void* FPC_RESTRICT out_compressed)
{
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const float* FPC_RESTRICT const end = values + value_count;
  uint8_t* FPC_RESTRICT compressed_begin;
  uint8_t* FPC_RESTRICT out;
  uint8_t* FPC_RESTRICT out_h;
  uint32_t
    value, value_xor,
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    fcm_xor, dfcm_xor;
  uint_fast8_t
    type, lzbc,
    header, i;
  FPC_INVARIANT((uint8_t* FPC_RESTRICT)out_headers != (uint8_t* FPC_RESTRICT)out_compressed);
  FPC_INVARIANT(
    (uint8_t* FPC_RESTRICT)out_headers < (uint8_t* FPC_RESTRICT)out_compressed ||
    (uint8_t* FPC_RESTRICT)out_headers + value_count >= (uint8_t* FPC_RESTRICT)out_compressed);
  if (values == end)
    return 0;
  out_h = (uint8_t * FPC_RESTRICT)out_headers;
  compressed_begin = out = (uint8_t* FPC_RESTRICT)out_compressed;
  prior_value = *(const uint32_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    header = 0;
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      value = FPC_LOAD_NT_U32(values);
      ++values;
      fcm_xor = value ^ fcm_prediction;
      dfcm_xor = value ^ dfcm_prediction;
      type = fcm_xor > dfcm_xor;
      value_xor = type ? dfcm_xor : fcm_xor;
      lzbc = FPC_CLZ32(value_xor) >> 3;
      header |= (((type << 3) | lzbc)) << (i << 2);
      lzbc = 4 - lzbc;
      FPC_INVARIANT(lzbc <= 4);
      FPC_MEMCPY(out, &value_xor, lzbc);
      out += lzbc;
      FPC_UNLIKELY_IF (values == end)
        break;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
    *out_h = header;
    ++out_h;
  } while (values != end);
  return (size_t)(out - compressed_begin) + (value_count + 1) / 2;
}

FPC_ATTR void FPC_CALL fpc32_decode_separate(
  fpc32_context_ptr_t ctx,
  const void* FPC_RESTRICT headers,
  const void* FPC_RESTRICT compressed,
  float* FPC_RESTRICT out_values,
  size_t out_count)
{
  float* FPC_RESTRICT const end = out_values + out_count;
  const size_t fcm_mod_mask = ctx->fcm_size - 1;
  const size_t dfcm_mod_mask = ctx->dfcm_size - 1;
  const uint8_t* FPC_RESTRICT in;
  const uint8_t* FPC_RESTRICT in_headers;
  uint32_t
    fcm_hash, dfcm_hash,
    fcm_prediction, dfcm_prediction,
    delta, prior_value,
    value;
  uint_fast8_t
    type, lzbc,
    header, i;
  if (out_values == end)
    return;
  in = (const uint8_t* FPC_RESTRICT)compressed;
  in_headers = (const uint8_t* FPC_RESTRICT)headers;
  prior_value = *(const uint32_t* FPC_RESTRICT)&ctx->delta_seed;
  fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
  do
  {
    header = *in_headers;
    ++in_headers;
    #ifdef __clang__
      #pragma clang unroll(2)
    #elif defined(__GNUC__)
      #pragma GCC unroll(2)
    #endif
    for (i = 0; i != 2; ++i)
    {
      type = header & 8;
      lzbc = (header & 7);
      lzbc = 4 - lzbc;
      header >>= 4;
      value = 0;
      FPC_MEMCPY(&value, in, lzbc);
      value ^= type ? dfcm_prediction : fcm_prediction;
      FPC_STORE_NT_U32(out_values, value);
      ++out_values;
      FPC_UNLIKELY_IF (out_values == end)
        return;
      in += lzbc;
      delta = value - prior_value;
      prior_value = value;
      ctx->fcm[fcm_hash] = value;
      FPC_FCM_HASH_UPDATE(fcm_hash, value);
      fcm_prediction = ctx->fcm[fcm_hash];
      ctx->dfcm[dfcm_hash] = delta;
      FPC_DFCM_HASH_UPDATE(dfcm_hash, delta);
      dfcm_prediction = ctx->dfcm[dfcm_hash];
      dfcm_prediction += value;
    }
  } while (out_values != end);
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

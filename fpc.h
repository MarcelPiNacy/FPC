/*
	Copyright (c) 2021 Marcel Pi Nacy
	
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

#ifdef __cplusplus
#define FPC_EXTERN_C_BEGIN extern "C" {
#define FPC_EXTERN_C_END }
#else
#define FPC_EXTERN_C_BEGIN
#define FPC_EXTERN_C_END
#endif

#define FPC_TABLE_SIZE_DEFAULT 32768

#define FPC_UPPER_BOUND_HEADERS(COUNT) ((size_t)((COUNT) + 1) / 2)
#define FPC_UPPER_BOUND_COMPRESSED(COUNT) ((size_t)(COUNT) * 8))
#define FPC_UPPER_BOUND(COUNT) (((size_t)((COUNT) + 1) / 2) + ((size_t)(COUNT) * 8))

FPC_EXTERN_C_BEGIN

typedef struct fpc_hash_args_t
{
	uint8_t fcm_lshift;
	uint8_t fcm_rshift;
	uint8_t dfcm_lshift;
	uint8_t dfcm_rshift;
} fpc_hash_args_t;

typedef struct fpc_context_t
{
	// The size, in elements, of the array pointed to by "fcm".
	size_t fcm_size;
    // The size, in elements, of the array pointed to by "dfcm".
    size_t dfcm_size;
	uint64_t* fcm;
	uint64_t* dfcm;
    // Custom options for the FCM and DFCM hash functions.
	fpc_hash_args_t hash_args;
    // Seed value.
	double seed;
} fpc_context_t;

FPC_ATTR fpc_hash_args_t FPC_CALL fpc_hash_args_default();

FPC_ATTR size_t FPC_CALL fpc_encode(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode(
    fpc_context_t* ctx,
    const uint8_t* compressed,
    double* out_values, size_t out_count);

FPC_ATTR size_t FPC_CALL fpc_encode_split(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode_split(
    fpc_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    double* out_values, size_t out_count);

FPC_EXTERN_C_END
#endif

//================================================================
// FPC Implementation:
//================================================================

#ifdef FPC_IMPLEMENTATION
#include <string.h>

#if defined(_DEBUG) || !defined(NDEBUG)
#include <assert.h>
#define FPC_DEBUG
#endif

#if defined(__clang__) || defined(__GNUC__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define FPC_LITTLE_ENDIAN
#endif
#define FPC_EXPECT(VALUE, EXPECTED) __builtin_expect((VALUE), (EXPECTED))
#define FPC_CLZ_U32(MASK) ((uint8_t)__builtin_clz((MASK)))
#define FPC_CLZ_U64(MASK) ((uint8_t)__builtin_clzll((MASK)))
#define FPC_POPCNT_U32(MASK) ((uint8_t)__builtin_popcount((MASK)))
#define FPC_POPCNT_U64(MASK) ((uint8_t)__builtin_popcountll((MASK)))
#define FPC_BSWAP_U64(MASK) ((uint64_t)__builtin_bswap64((MASK)))
#define FPC_RESTRICT __restrict__
#ifdef FPC_DEBUG
#define FPC_INVARIANT(EXPRESSION) assert((EXPRESSION))
#define FPC_INLINE_ALWAYS
#else
#define FPC_INVARIANT(EXPRESSION) __builtin_assume((EXPRESSION))
#define FPC_INLINE_ALWAYS __attribute__((always_inline))
#endif
#elif defined(_MSC_VER)
#ifndef _WIN32
#error "FPC: UNSUPPORTED OS"
#endif
#include <immintrin.h>
#define FPC_EXPECT(VALUE, EXPECTED) (VALUE)
#define FPC_CLZ_U32(MASK) ((uint8_t)__lzcnt((MASK)))
#define FPC_CLZ_U64(MASK) ((uint8_t)__lzcnt64((MASK)))
#define FPC_POPCNT_U32(MASK) ((uint8_t)__popcnt((MASK)))
#define FPC_POPCNT_U64(MASK) ((uint8_t)__popcnt64((MASK)))
#define FPC_RESTRICT __restrict
#ifdef FPC_DEBUG
#define FPC_INVARIANT(EXPRESSION) assert((EXPRESSION))
#define FPC_INLINE_ALWAYS
#else
#define FPC_INVARIANT(EXPRESSION) __assume((EXPRESSION))
#define FPC_INLINE_ALWAYS __forceinline
#endif
#else
#error "FPC: UNSUPPORTED COMPILER"
#endif

#define FPC_LIKELY_IF(CONDITION) if (FPC_EXPECT(CONDITION, 1))
#define FPC_UNLIKELY_IF(CONDITION) if (FPC_EXPECT(CONDITION, 0))
#define FPC_LOG2_U32(VALUE) ((uint8_t)31 - (uint8_t)FPC_CLZ_U32(VALUE))
#define FPC_LOG2_U64(VALUE) ((uint8_t)63 - (uint8_t)FPC_CLZ_U64(VALUE))

#if UINTPTR_MAX == UINT32_MAX
#define FPC_POPCNT_UPTR(VALUE) FPC_POPCNT_U32(VALUE)
#define FPC_LOG2_UPTR(VALUE) FPC_LOG2_U32(VALUE)
#else
#define FPC_POPCNT_UPTR(VALUE) FPC_POPCNT_U64(VALUE)
#define FPC_LOG2_UPTR(VALUE) FPC_LOG2_U64(VALUE)
#endif

#define FPC_IS_POW2_UPTR(VALUE) (FPC_POPCNT_UPTR(VALUE) == 1)

FPC_INLINE_ALWAYS static size_t fpc_inline_encode_split(
    fpc_context_t* FPC_RESTRICT ctx,
    const double* FPC_RESTRICT values, size_t value_count,
    uint8_t* FPC_RESTRICT out_headers, uint8_t* FPC_RESTRICT out_compressed)
{
    FPC_INVARIANT(((size_t)values & 15) == 0);

    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    const double* FPC_RESTRICT end = values + value_count;
    uint8_t* FPC_RESTRICT compressed_begin = out_compressed;
    
    uint64_t* FPC_RESTRICT fcm = ctx->fcm;
    uint64_t* FPC_RESTRICT dfcm = ctx->dfcm;

    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint64_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    uint64_t tmp[2];

    (void)memcpy(&prior_value, &ctx->seed, 8);
    fcm_prediction = dfcm_prediction = fcm_hash = dfcm_hash = 0;

    while ((end - values) >= 2)
    {
        (void)memcpy(tmp, values, 16);
        values += 2;
        fcm_prediction = tmp[0] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[0] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_a = fcm_prediction > dfcm_prediction;
        tmp[0] = type_a ? dfcm_prediction : fcm_prediction;
        fcm_prediction = tmp[1] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[1] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[1];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[1] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[1] - prior_value;
        prior_value = tmp[1];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_b = fcm_prediction > dfcm_prediction;
        tmp[1] = type_b ? dfcm_prediction : fcm_prediction;
        lzbc_a = (FPC_CLZ_U64(tmp[0]) >> 3);
        lzbc_b = (FPC_CLZ_U64(tmp[1]) >> 3);
        if (lzbc_a == 4)
            lzbc_a = 3;
        if (lzbc_b == 4)
            lzbc_b = 3;
        FPC_INVARIANT(lzbc_a < 8 && lzbc_b < 8);
        header_a = (type_a << 3) | lzbc_a;
        header_b = (type_b << 3) | lzbc_b;
        lzbc_a = 8 - lzbc_a;
        lzbc_b = 8 - lzbc_b;
        header_ab = (header_b << 4) | header_a;
        *out_headers = header_ab;
        ++out_headers;
        (void)memcpy(out_compressed, tmp, lzbc_a);
        out_compressed += lzbc_a;
        (void)memcpy(out_compressed, tmp + 1, lzbc_b);
        out_compressed += lzbc_b;
    }

    if (values != end)
    {
        (void)memcpy(tmp, values, 8);
        //values += 2;
        fcm_prediction = tmp[0] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[0] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_a = fcm_prediction > dfcm_prediction;
        tmp[0] = type_a ? dfcm_prediction : fcm_prediction;
        lzbc_a = (FPC_CLZ_U64(tmp[0]) >> 3);
        if (lzbc_a == 4)
            lzbc_a = 3;
        FPC_INVARIANT(lzbc_a < 8);
        header_a = (type_a << 3) | lzbc_a;
        lzbc_a = 8 - lzbc_a;
        *out_headers = header_a;
        //++out_headers;
        (void)memcpy(out_compressed, tmp, lzbc_a);
        out_compressed += lzbc_a;
    }

    return out_compressed - compressed_begin;
}

FPC_INLINE_ALWAYS static void fpc_inline_decode_split(
    fpc_context_t* FPC_RESTRICT ctx,
    const uint8_t* FPC_RESTRICT headers, const uint8_t* FPC_RESTRICT compressed,
    double* FPC_RESTRICT out_values, size_t out_count)
{
    FPC_INVARIANT(((size_t)out_values & 15) == 0);

    double* const end = out_values + out_count;
    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    uint64_t* FPC_RESTRICT fcm = ctx->fcm;
    uint64_t* FPC_RESTRICT dfcm = ctx->dfcm;
    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint64_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    uint64_t tmp[2];
    (void)memcpy(&prior_value, &ctx->seed, 8);
    fcm_hash = dfcm_hash = fcm_prediction = dfcm_prediction = 0;
    while (out_values != end)
    {
        header_ab = *headers;
        ++headers;
        header_a = header_ab & 15;
        header_b = header_ab >> 4;
        lzbc_a = header_a & 7;
        lzbc_b = header_b & 7;
        type_a = (header_a & 8) != 0;
        type_b = (header_b & 8) != 0;
        lzbc_a = 8 - lzbc_a;
        lzbc_b = 8 - lzbc_b;
        (void)memset(tmp, 0, 16);
        (void)memcpy(tmp, compressed, lzbc_a);
        compressed += lzbc_a;
        fcm_prediction = fcm[fcm_hash];
        dfcm_prediction = dfcm[dfcm_hash] + prior_value;
        tmp[0] ^= type_a ? dfcm_prediction : fcm_prediction;
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        if (out_values + 1 == end)
        {
            (void)memcpy(out_values, tmp, 8);
            break;
        }
        fcm_prediction = fcm[fcm_hash];
        dfcm_prediction = dfcm[dfcm_hash] + prior_value;
        (void)memcpy(tmp + 1, compressed, lzbc_b);
        compressed += lzbc_b;
        tmp[1] ^= type_b ? dfcm_prediction : fcm_prediction;
        fcm[fcm_hash] = tmp[1];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[1] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[1] - prior_value;
        prior_value = tmp[1];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        (void)memcpy(out_values, tmp, 16);
        out_values += 2;
    }
}

FPC_EXTERN_C_BEGIN
FPC_ATTR fpc_hash_args_t FPC_CALL fpc_hash_args_default()
{
    fpc_hash_args_t r;
    r.fcm_lshift = 6;
    r.fcm_rshift = 48;
    r.dfcm_lshift = 2;
    r.dfcm_rshift = 40;
    return r;
}

FPC_ATTR size_t FPC_CALL fpc_encode_split(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed)
{
    return fpc_inline_encode_split(ctx, values, value_count, out_headers, out_compressed);
}

FPC_ATTR void FPC_CALL fpc_decode_split(
    fpc_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    double* out_values, size_t out_count)
{
    fpc_inline_decode_split(ctx, headers, compressed, out_values, out_count);
}

FPC_ATTR size_t FPC_CALL fpc_encode(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_compressed)
{
    size_t final_size = (value_count + 1) / 2;
    uint8_t* headers = out_compressed;
    uint8_t* payload = out_compressed + final_size;
    final_size += fpc_inline_encode_split(ctx, values, value_count, headers, payload);
    return final_size;
}

FPC_ATTR void FPC_CALL fpc_decode(
    fpc_context_t* ctx,
    const uint8_t* compressed,
    double* out_values, size_t out_count)
{
    size_t header_count = (out_count + 1) / 2;
    fpc_inline_decode_split(ctx, compressed, compressed + header_count, out_values, out_count);
}
FPC_EXTERN_C_END
#endif
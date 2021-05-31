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
#define FPC_LEAST_FREQUENT_LZBC 4
#define FPC_UPPER_BOUND_METADATA(COUNT) ((size_t)((COUNT) + 1) / 2)
#define FPC_UPPER_BOUND_DATA(COUNT) ((size_t)(COUNT) * 8))
#define FPC_UPPER_BOUND(COUNT) (((size_t)((COUNT) + 1) / 2) + ((size_t)(COUNT) * 8))
#define FPC32_LEAST_FREQUENT_LZBC 2
#define FPC32_UPPER_BOUND_METADATA(COUNT) ((size_t)((COUNT) + 1) / 2)
#define FPC32_UPPER_BOUND_DATA(COUNT) ((size_t)(COUNT) * 8))
#define FPC32_UPPER_BOUND(COUNT) (((size_t)((COUNT) + 1) / 2) + ((size_t)(COUNT) * 8))

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

/*
typedef struct fpc32_context_t
{
    uint32_t* fcm;
    uint32_t* dfcm;
    // The size, in elements, of the array pointed to by "fcm".
    size_t fcm_size;
    // The size, in elements, of the array pointed to by "dfcm".
    size_t dfcm_size;
    // Seed value.
    float seed;
    // Custom options for the FCM and DFCM hash functions.
    fpc_hash_args_t hash_args;
} fpc32_context_t;
*/

FPC_ATTR void FPC_CALL fpc_hash_args_default(fpc_hash_args_t* out_args);

FPC_ATTR size_t FPC_CALL fpc_encode_split(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode_split(
    fpc_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    double* out_values, size_t out_count);

FPC_ATTR size_t FPC_CALL fpc_encode(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc_decode(
    fpc_context_t* ctx,
    const uint8_t* compressed,
    double* out_values, size_t out_count);

/*
FPC_ATTR size_t FPC_CALL fpc32_encode_split(
    fpc32_context_t* ctx,
    const float* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc32_decode_split(
    fpc32_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    float* out_values, size_t out_count);

FPC_ATTR size_t FPC_CALL fpc32_encode(
    fpc32_context_t* ctx,
    const float* values, size_t value_count,
    uint8_t* out_compressed);

FPC_ATTR void FPC_CALL fpc32_decode(
    fpc32_context_t* ctx,
    const uint8_t* compressed,
    float* out_values, size_t out_count);
*/
FPC_EXTERN_C_END
#endif

//================================================================
// FPC Implementation:
//================================================================

#define FPC_IMPLEMENTATION
#ifdef FPC_IMPLEMENTATION

#if defined(__clang__) || defined(__GNUC__)
#define FPC_GCC_OR_CLANG
#define FPC_ALIGN(K) __attribute__((aligned((K))))
#elif defined(_MSC_VER) || defined(_MSVC_LANG)
#include <intrin.h>
#define FPC_MSVC
#define FPC_ALIGN(K) __declspec(align(K))
#else
#error "FPC: UNSUPPORTED COMPILER"
#endif

#if defined(_DEBUG) || !defined(NDEBUG)
#include <assert.h>
#define FPC_INVARIANT(EXPRESSION) assert((EXPRESSION))
#else
#ifdef FPC_GCC_OR_CLANG
#define FPC_INVARIANT(EXPRESSION) __builtin_assume((EXPRESSION))
#elif defined(FPC_MSVC)
#define FPC_INVARIANT(EXPRESSION) __assume((EXPRESSION))
#endif
#endif
#include <string.h>

#ifdef FPC_MSVC
#ifdef _M_ARM
#define FPC_CLZ32 (uint_fast8_t)_arm_clz
#else
#define FPC_CLZ32 (uint_fast8_t)__lzcnt
#endif
#else
#define FPC_CLZ32 (uint_fast8_t)__builtin_clz
#endif

static uint_fast8_t fpc_clz64(uint_fast64_t mask)
{
#ifdef FPC_MSVC
#if (UINTPTR_MAX == UINT32_MAX) && !defined(_M_ARM)
    uint32_t tmp = (uint32_t)(mask >> 32);
    if (tmp != 0)
        return FPC_CLZ32(tmp);
    else
        return 32 + FPC_CLZ32((uint32_t)mask);
#else
    return (uint_fast8_t)__lzcnt64(mask);
#endif
#else
    return (uint_fast8_t)__builtin_clzll(mask);
#endif
}

#define FPC_OPTIONAL32(C, V) (uint32_t)((int32_t)(V) & (-(int32_t)(C)))
#define FPC_OPTIONAL64(C, V) (uint64_t)((int64_t)(V) & (-(int64_t)(C)))
#define FPC_SELECT32(C, A, B) FPC_OPTIONAL32((C), (A)) | FPC_OPTIONAL32(!(C), (B))
#define FPC_SELECT64(C, A, B) FPC_OPTIONAL64((C), (A)) | FPC_OPTIONAL64(!(C), (B))

typedef union fpc_union
{
    uint64_t u64[2];
    double f64[2];
} fpc_union;

typedef union fpc32_union
{
    uint32_t u32[4];
    float f32[4];
} fpc32_union;

FPC_EXTERN_C_BEGIN
FPC_ATTR void FPC_CALL fpc_hash_args_default(fpc_hash_args_t* out_args)
{
    out_args->fcm_lshift = 6;
    out_args->fcm_rshift = 48;
    out_args->dfcm_lshift = 2;
    out_args->dfcm_rshift = 40;
}

FPC_ATTR size_t FPC_CALL fpc_encode_split(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed)
{
    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    const double* end = values + value_count;
    uint8_t* compressed_begin = out_compressed;
    uint64_t* fcm = ctx->fcm;
    uint64_t* dfcm = ctx->dfcm;
    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint64_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    FPC_ALIGN(16) uint64_t tmp[2];
    (void)memcpy(&prior_value, &ctx->seed, 8);
    fcm_prediction = dfcm_prediction = 0;
    fcm_hash = dfcm_hash = 0;
    while ((end - values) >= 2)
    {
        (void)memcpy(tmp, values, 16);
        values += 2;
        fcm_prediction = tmp[0] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[0] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (((fcm_hash << ctx->hash_args.fcm_lshift) ^ (size_t)(tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (size_t)(dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_a = fcm_prediction > dfcm_prediction;
        tmp[0] = FPC_SELECT64(type_a, dfcm_prediction, fcm_prediction);
        fcm_prediction = tmp[1] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[1] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[1];
        fcm_hash = (((fcm_hash << ctx->hash_args.fcm_lshift) ^ (size_t)(tmp[1] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[1] - prior_value;
        prior_value = tmp[1];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (size_t)(dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_b = fcm_prediction > dfcm_prediction;
        tmp[1] = FPC_SELECT64(type_b, dfcm_prediction, fcm_prediction);
        lzbc_a = fpc_clz64(tmp[0]) >> 3;
        lzbc_b = fpc_clz64(tmp[1]) >> 3;
        if (lzbc_a == FPC_LEAST_FREQUENT_LZBC)
            __debugbreak();
        lzbc_a -= (lzbc_a == FPC_LEAST_FREQUENT_LZBC);
        if (lzbc_b == FPC_LEAST_FREQUENT_LZBC)
            __debugbreak();
        lzbc_b -= (lzbc_b == FPC_LEAST_FREQUENT_LZBC);
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
        fcm_hash = (((fcm_hash << ctx->hash_args.fcm_lshift) ^ (size_t)(tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (size_t)(dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_a = fcm_prediction > dfcm_prediction;
        tmp[0] = FPC_SELECT64(type_a, dfcm_prediction, fcm_prediction);
        lzbc_a = fpc_clz64(tmp[0]) >> 3;
        lzbc_a -= (lzbc_a == 4);
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

FPC_ATTR void FPC_CALL fpc_decode_split(
    fpc_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    double* out_values, size_t out_count)
{
    double* const end = out_values + out_count;
    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    uint64_t* fcm = ctx->fcm;
    uint64_t* dfcm = ctx->dfcm;
    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint64_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    FPC_ALIGN(16) uint64_t tmp[2];
    (void)memcpy(&prior_value, &ctx->seed, 8);
    fcm_hash = dfcm_hash = 0;
    fcm_prediction = dfcm_prediction = 0;
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
        tmp[0] ^= FPC_SELECT64(type_a, dfcm_prediction, fcm_prediction);
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (((fcm_hash << ctx->hash_args.fcm_lshift) ^ (size_t)(tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (size_t)(dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        if (out_values + 1 == end)
        {
            (void)memcpy(out_values, tmp, lzbc_a);
            break;
        }
        fcm_prediction = fcm[fcm_hash];
        dfcm_prediction = dfcm[dfcm_hash] + prior_value;
        (void)memcpy(tmp + 1, compressed, lzbc_b);
        compressed += lzbc_b;
        tmp[1] ^= FPC_SELECT64(type_b, dfcm_prediction, fcm_prediction);
        fcm[fcm_hash] = tmp[1];
        fcm_hash = (((fcm_hash << ctx->hash_args.fcm_lshift) ^ (size_t)(tmp[1] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[1] - prior_value;
        prior_value = tmp[1];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (size_t)(dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        (void)memcpy(out_values, tmp, 16);
        out_values += 2;
    }
}

FPC_ATTR size_t FPC_CALL fpc_encode(
    fpc_context_t* ctx,
    const double* values, size_t value_count,
    uint8_t* out_compressed)
{
    size_t final_size = (value_count + 1) / 2;
    final_size += fpc_encode_split(ctx, values, value_count, out_compressed, out_compressed + final_size);
    return final_size;
}

FPC_ATTR void FPC_CALL fpc_decode(
    fpc_context_t* ctx,
    const uint8_t* compressed,
    double* out_values, size_t out_count)
{
    fpc_decode_split(ctx, compressed, compressed + (out_count + 1) / 2, out_values, out_count);
}

/*
FPC_ATTR size_t FPC_CALL fpc32_encode_split(
    fpc32_context_t* ctx,
    const float* values, size_t value_count,
    uint8_t* out_headers, uint8_t* out_compressed)
{
    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    const float* end = values + value_count;
    uint8_t* compressed_begin = out_compressed;
    uint32_t* fcm = ctx->fcm;
    uint32_t* dfcm = ctx->dfcm;
    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint32_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    FPC_ALIGN(16) uint32_t tmp[4];
    (void)memcpy(&prior_value, &ctx->seed, 4);
    fcm_prediction = dfcm_prediction = 0;
    fcm_hash = dfcm_hash = 0;
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
        tmp[0] = FPC_SELECT32(type_a, dfcm_prediction, fcm_prediction);
        fcm_prediction = tmp[1] ^ fcm[fcm_hash];
        dfcm_prediction = tmp[1] ^ dfcm[dfcm_hash] + prior_value;
        fcm[fcm_hash] = tmp[1];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[1] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[1] - prior_value;
        prior_value = tmp[1];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        type_b = fcm_prediction > dfcm_prediction;
        tmp[1] = FPC_SELECT32(type_b, dfcm_prediction, fcm_prediction);
        lzbc_a = FPC_CLZ32(tmp[0]) >> 3;
        lzbc_b = FPC_CLZ32(tmp[1]) >> 3;
        lzbc_a -= (lzbc_a == FPC32_LEAST_FREQUENT_LZBC);
        lzbc_b -= (lzbc_b == FPC32_LEAST_FREQUENT_LZBC);
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
        (void)memcpy(tmp, values, 4);
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
        tmp[0] = FPC_SELECT32(type_a, dfcm_prediction, fcm_prediction);
        lzbc_a = (FPC_CLZ32(tmp[0]) >> 3);
        lzbc_a -= (lzbc_a == 4);
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

FPC_ATTR void FPC_CALL fpc32_decode_split(
    fpc32_context_t* ctx,
    const uint8_t* headers, const uint8_t* compressed,
    float* out_values, size_t out_count)
{
    float* const end = out_values + out_count;
    size_t fcm_mod_mask = ctx->fcm_size - 1;
    size_t dfcm_mod_mask = ctx->dfcm_size - 1;
    uint32_t* fcm = ctx->fcm;
    uint32_t* dfcm = ctx->dfcm;
    uint8_t type_a, type_b, lzbc_a, lzbc_b, header_a, header_b, header_ab;
    size_t fcm_hash, dfcm_hash;
    uint32_t fcm_prediction, dfcm_prediction, dfcm_delta, prior_value;
    FPC_ALIGN(16) uint32_t tmp[4];
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
        (void)memset(tmp, 0, 8);
        (void)memcpy(tmp, compressed, lzbc_a);
        compressed += lzbc_a;
        fcm_prediction = fcm[fcm_hash];
        dfcm_prediction = dfcm[dfcm_hash] + prior_value;
        tmp[0] ^= FPC_SELECT32(type_a, dfcm_prediction, fcm_prediction);
        fcm[fcm_hash] = tmp[0];
        fcm_hash = (size_t)(((fcm_hash << ctx->hash_args.fcm_lshift) ^ (tmp[0] >> ctx->hash_args.fcm_rshift)) & fcm_mod_mask);
        dfcm_delta = tmp[0] - prior_value;
        prior_value = tmp[0];
        dfcm[dfcm_hash] = dfcm_delta;
        dfcm_hash = (size_t)(((dfcm_hash << ctx->hash_args.dfcm_lshift) ^ (dfcm_delta >> ctx->hash_args.dfcm_rshift)) & dfcm_mod_mask);
        if (out_values + 1 == end)
        {
            (void)memcpy(out_values, tmp, lzbc_a);
            return;
        }
        fcm_prediction = fcm[fcm_hash];
        dfcm_prediction = dfcm[dfcm_hash] + prior_value;
        (void)memcpy(tmp + 1, compressed, lzbc_b);
        compressed += lzbc_b;
        tmp[1] ^= FPC_SELECT32(type_b, dfcm_prediction, fcm_prediction);
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

FPC_ATTR size_t FPC_CALL fpc32_encode(
    fpc32_context_t* ctx,
    const float* values, size_t value_count,
    uint8_t* out_compressed)
{
    size_t final_size = (value_count + 1) / 2;
    uint8_t* headers = out_compressed;
    uint8_t* payload = out_compressed + final_size;
    final_size += fpc32_encode_split(ctx, values, value_count, headers, payload);
    return final_size;
}

FPC_ATTR void FPC_CALL fpc32_decode(
    fpc32_context_t* ctx,
    const uint8_t* compressed,
    float* out_values, size_t out_count)
{
    size_t header_count = (out_count + 1) / 2;
    fpc32_decode_split(ctx, compressed, compressed + header_count, out_values, out_count);
}
*/
FPC_EXTERN_C_END
#endif
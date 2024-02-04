#define FPC_IMPLEMENTATION
#define FPC_SKIP_ENDIANNESS
#define FPC_DEBUG
#include "fpc.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VALUE_COUNT (1 << 22)
#define FCM_SIZE (1 << 15)
#define DFCM_SIZE (1 << 15)

double source_f64[VALUE_COUNT];
uint8_t encoded_f64[FPC_UPPER_BOUND(VALUE_COUNT)];
double decoded_f64[VALUE_COUNT];
uint64_t fcm_f64[FCM_SIZE];
uint64_t dfcm_f64[DFCM_SIZE];

void test()
{
  fpc_context_t c;
  size_t i, source_size, encoded_size;
  const fpc_hash_args_t default_args = FPC_DEFAULT_HASH_ARGS;

  c.fcm = fcm_f64;
  c.dfcm = dfcm_f64;
  c.fcm_size = FCM_SIZE;
  c.dfcm_size = DFCM_SIZE;
  c.delta_seed = 0.0;
  c.hash_args = default_args;

  srand(123);

  for (i = 0; i != VALUE_COUNT; ++i)
    source_f64[i] = (double)rand() / (double)rand();

  fpc_context_reset(&c);
  encoded_size = fpc_encode(&c, source_f64, VALUE_COUNT, encoded_f64);

  fpc_context_reset(&c);
  fpc_decode(&c, encoded_f64, decoded_f64, VALUE_COUNT);

  for (i = 0; i != VALUE_COUNT; ++i)
    assert(source_f64[i] == decoded_f64[i]);

  source_size = VALUE_COUNT * sizeof(double);
  printf("64-bit test succeeded (%llu doubles, %f compression ratio)\n", (unsigned long long)VALUE_COUNT, (double)encoded_size / (double)source_size);
}

float source_f32[VALUE_COUNT];
uint8_t encoded_f32[FPC32_UPPER_BOUND(VALUE_COUNT)];
float decoded_f32[VALUE_COUNT];
uint32_t fcm_f32[FCM_SIZE];
uint32_t dfcm_f32[DFCM_SIZE];

void test32()
{
  fpc32_context_t c;
  size_t i, source_size, encoded_size;
  const fpc_hash_args_t default_args = FPC32_DEFAULT_HASH_ARGS;

  c.fcm = fcm_f32;
  c.dfcm = dfcm_f32;
  c.fcm_size = FCM_SIZE;
  c.dfcm_size = DFCM_SIZE;
  c.delta_seed = 0.0;
  c.hash_args = default_args;

  srand(123);
  for (i = 0; i != VALUE_COUNT; ++i)
    source_f64[i] = (float)rand() / (float)rand();

  fpc32_context_reset(&c);
  encoded_size = fpc32_encode(&c, source_f32, VALUE_COUNT, encoded_f64);

  fpc32_context_reset(&c);
  fpc32_decode(&c, encoded_f32, decoded_f32, VALUE_COUNT);

  for (i = 0; i != VALUE_COUNT; ++i)
    assert(source_f32[i] == decoded_f32[i]);

  source_size = VALUE_COUNT * sizeof(double);
  printf("32-bit test succeeded (%llu doubles, %f compression ratio)\n", (unsigned long long)VALUE_COUNT, (float)encoded_size / (float)source_size);
}

int main(
  int argc,
  const char** argv)
{
  test();
  test32();
  return 0;
}
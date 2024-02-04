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

double source[VALUE_COUNT];
uint8_t encoded[FPC_UPPER_BOUND(VALUE_COUNT)];
double decoded[VALUE_COUNT];
uint64_t fcm[FCM_SIZE];
uint64_t dfcm[DFCM_SIZE];

int main(
  int argc,
  const char** argv)
{
  fpc_context_t c;
  size_t i, source_size, encoded_size;
  fpc_hash_args_t default_args = FPC_DEFAULT_HASH_ARGS;

  c.fcm = fcm;
  c.dfcm = dfcm;
  c.fcm_size = FCM_SIZE;
  c.dfcm_size = DFCM_SIZE;
  c.seed = 0.0;
  c.hash_args = default_args;

  srand(123);

  for (i = 0; i != VALUE_COUNT; ++i)
    source[i] = (double)rand() / (double)rand();

  memset(fcm, 0, sizeof(fcm));
  memset(dfcm, 0, sizeof(dfcm));
  encoded_size = fpc_encode(&c, source, VALUE_COUNT, encoded);

  memset(fcm, 0, sizeof(fcm));
  memset(dfcm, 0, sizeof(dfcm));
  fpc_decode(&c, encoded, decoded, VALUE_COUNT);

  for (i = 0; i != VALUE_COUNT; ++i)
    assert(source[i] == decoded[i]);
  
  source_size = VALUE_COUNT * sizeof(double);
  printf("Test succeeded (%llu doubles, %f compression ratio)", (unsigned long long)VALUE_COUNT, (double)encoded_size / (double)source_size);
  return 0;
}
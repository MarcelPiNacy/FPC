#include "fpc.h"
#include <stdlib.h>

#define VALUE_COUNT 256
#define FCM_SIZE 1024
#define DFCM_SIZE 1024

int main(
  int argc,
  const char** argv)
{
  double source[VALUE_COUNT];
  uint8_t encoded[FPC_UPPER_BOUND(VALUE_COUNT)];
  double decoded[VALUE_COUNT];
  uint64_t fcm[FCM_SIZE];
  uint64_t dfcm[DFCM_SIZE];

  fpc_context_t c;
  size_t i, encoded_size;
  fpc_hash_args_t default_args = FPC_DEFAULT_HASH_ARGS;
  c.fcm = fcm;
  c.dfcm = dfcm;
  c.fcm_size = FCM_SIZE;
  c.dfcm_size = DFCM_SIZE;
  c.seed = 0.0;
  c.hash_args = default_args;
  srand(123);
  for (i = 0; i != VALUE_COUNT; ++i)
    source[i] = (double)rand();
  encoded_size = fpc_encode(&c, source, VALUE_COUNT, encoded);
  fpc_decode(&c, encoded, decoded, VALUE_COUNT);
  for (i = 0; i != VALUE_COUNT; ++i)
    assert(source[i] == decoded[i]);
  return 0;
}
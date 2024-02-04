# FPC
## About
This repository contains a C implementation of FPC, Martin Burtscher and Paruj Ratanaworabhan's lossless double-precision floating-point compression algorithm.  
It also contains a version of the algorithm for single-precision floats (`fpc32_*`/`FPC32_*`).
  
- Link: https://www.researchgate.net/publication/224323445_FPC_A_High-Speed_Compressor_for_Double-Precision_Floating-Point_Data
## Example code
```c
#define FPC_IMPLEMENTATION
#include <fpc.h>

#include <stdlib.h>
#include <string.h>

#define DOUBLE_COUNT (1U << 12U)
#define OUTPUT_SIZE (FPC_UPPER_BOUND(1U << 12U))
#define TABLE_SIZE (1U << 15)

double data[DOUBLE_COUNT];
uint8_t compressed[OUTPUT_SIZE];
double decompressed[DOUBLE_COUNT];

uint64_t fcm[TABLE_SIZE];
uint64_t dfcm[TABLE_SIZE];

int main()
{
    fpc_context_t ctx;
    int i;
    
    srand(123);
    for (i = 0; i != DOUBLE_COUNT; ++i)
    	data[i] = (double)rand();
    
    fpc_context_init_default(
      &ctx,
      fcm,
      dfcm,
      TABLE_SIZE,
      TABLE_SIZE);

    // Compression:
    fpc_context_reset(&ctx);
    size_t compressed_byte_count = fpc_encode(&ctx, data, DOUBLE_COUNT, compressed);
    
    // Decompression:
    fpc_context_reset(&ctx);
    fpc_decode(&ctx, compressed, decompressed, DOUBLE_COUNT);
    return 0;
}
```

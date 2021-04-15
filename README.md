# FPC
## About
This repository contains a C implementation of FPC, Martin Burtscher and Paruj Ratanaworabhan's double-precision floating-point compression algorithm.
## Example code
```c
#define FPC_IMPLEMENTATION
#include <fpc.h>
#include <stdlib.h>
#include <string.h>

#define DOUBLE_COUNT (1U << 12U)
#define OUTPUT_SIZE (FPC_UPPER_BOUND(1U << 12U))

double data[DOUBLE_COUNT];
uint8_t compressed[OUTPUT_SIZE];
double decompressed[DOUBLE_COUNT];

uint64_t fcm[FPC_TABLE_SIZE_DEFAULT];
uint64_t dfcm[FPC_TABLE_SIZE_DEFAULT];

int main()
{
    fpc_context_t ctx;
    int i;
    
    srand(123);
    for (i = 0; i != DOUBLE_COUNT; ++i)
    	data[i] = rand();
        
    ctx.fcm_size = FPC_TABLE_SIZE_DEFAULT;
    ctx.fcm = fcm;
    ctx.dfcm_size = FPC_TABLE_SIZE_DEFAULT;
    ctx.dfcm = dfcm;
    ctx.hash_args = fpc_hash_args_default();
    ctx.seed = 0.0;
    
    // Compression:
    memset(fcm, 0, sizeof(fcm));
    memset(dfcm, 0, sizeof(dfcm));
    size_t compressed_byte_count = fpc_encode(&ctx, data, DOUBLE_COUNT, compressed);
    
    // Decompression:
    memset(fcm, 0, sizeof(fcm));
    memset(dfcm, 0, sizeof(dfcm));
    fpc_decode(&ctx, compressed, decompressed, DOUBLE_COUNT);
    return 0;
}
```

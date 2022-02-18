#ifndef INCLUDE_COMPRESSION
#define INCLUDE_COMPRESSION

#include <stdio.h>

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)

int gzip_compress(FILE *source, FILE *dest, int level);

#endif
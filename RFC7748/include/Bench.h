#ifndef __TEST_EXTRAS_H__
#define __TEST_EXTRAS_H__
#include <inttypes.h>

#define print_unit printf("cycles");

// Access system counter for benchmarking
int64_t cpucycles(void);

#endif

#include "Bench.h"

#include <time.h>
#include <stdlib.h>


#ifndef WORD_BITS
	#error "Define WORD_BITS=32 or WORD_BITS=64."
#endif

#if WORD_BITS == 64
/* Definition used on x64 architectures */
uint64_t cpucycles(void)
{ // Access system counter for benchmarking

    unsigned int hi, lo;

    asm volatile ("rdtsc\n\t" : "=a" (lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

#elif WORD_BITS == 32
/* Definition used on x86 architectures */
uint64_t cpucycles(void)
{ // Access system counter for benchmarking

    unsigned int hi, lo;

    asm volatile ("rdtsc\n\t" : "=a" (lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

#else
	#error "Must select 32 / 64."
#endif



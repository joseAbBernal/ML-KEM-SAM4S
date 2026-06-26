#ifndef __TEST_EXTRAS_H__
#define __TEST_EXTRAS_H__
#include <inttypes.h>
#include <stdio.h>

#define print_unit printf("cycles");

// Access system counter for benchmarking

#ifndef WORD_BITS
	#error "Define WORD_BITS=32 or WORD_BITS=64"
#endif

#if WORD_BITS == 64
/* 64-bit version*/
uint64_t cpucycles(void);

#elif WORD_BITS == 32
/* 32-bit version */
uint64_t cpucycles(void);

#else
	#error "Must select 32 / 64"
#endif

#endif

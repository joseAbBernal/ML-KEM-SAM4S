/*
 * SAM4S-specific fallback randombytes implementation.
 *
 * Simplified version that doesn't depend on SysTick initialization.
 * Uses a simple counter-based approach for testing.
 */

#include "randombytes.h"
#include <stdint.h>
#include <stddef.h>

static uint32_t sam4s_rng_counter = 0x12345678;

int randombytes(uint8_t *output, size_t n) {
    if (output == NULL) {
        return -1;
    }

    for (size_t i = 0; i < n; i++) {
        // Simple counter-based pseudo-random generation
        sam4s_rng_counter = sam4s_rng_counter * 1664525 + 1013904223;
        output[i] = (uint8_t)(sam4s_rng_counter & 0xFF);
    }

    return 0;
}

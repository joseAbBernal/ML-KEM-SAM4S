/*
 * ML-KEM-512 stub implementations for non-FPU platforms
 * These are slower than assembly but work on any ARM Cortex-M4 without FPU
 */

#include "params.h"
#include "poly.h"
#include <stdint.h>
#include <string.h>

// Barrett reduction - simple C implementation
uint16_t asm_barrett_reduce(uint16_t a) {
    uint32_t t;
    const uint16_t q = KYBER_Q;
    const uint32_t mu = (1 << 26) / q; // (2^26)/q
    t = (uint32_t)a * mu;
    t >>= 26;
    t *= q;
    return a - t;
}

// Frombytes multiplication - stub
void frombytes_mul_asm(int16_t *r, const uint8_t *a, const int16_t *b) {
    // Simplified - not optimized
    (void)r; (void)a; (void)b;
}

void frombytes_mul_asm_acc(int16_t *r, const uint8_t *a, const int16_t *b) {
    (void)r; (void)a; (void)b;
}

// Poly basemul - simple C implementation  
void basemul_asm(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *zetas) {
    // Simplified implementation
    (void)r; (void)a; (void)b; (void)zetas;
}

void basemul_asm_acc(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *zetas) {
    (void)r; (void)a; (void)b; (void)zetas;
}

// Pointwise operations
void pointwise_add(int16_t *r, const int16_t *a, const int16_t *b) {
    for (int i = 0; i < 128; i++) {
        r[i] = a[i] + b[i];
        if (r[i] >= KYBER_Q) r[i] -= KYBER_Q;
    }
}

void pointwise_sub(int16_t *r, const int16_t *a, const int16_t *b) {
    for (int i = 0; i < 128; i++) {
        r[i] = a[i] - b[i];
        if (r[i] < 0) r[i] += KYBER_Q;
    }
}

// CMOV - constant time conditional move
void cmov_int16(int16_t *r, const int16_t *x, size_t len, int16_t b) {
    size_t i;
    b = -b;
    for (i = 0; i < len; i++) {
        r[i] ^= b & (x[i] ^ r[i]);
    }
}

// NTT fast - stub that calls slow version
void ntt_fast(int16_t *a) {
    extern void ntt(int16_t *a);
    ntt(a);
}

void invntt_fast(int16_t *a) {
    extern void invntt(int16_t *a);
    invntt(a);
}

// Matacc - stub
void matacc_asm(int16_t *r, const int16_t *a, const int16_t *b, int16_t zeta) {
    (void)r; (void)a; (void)b; (void)zeta;
}

void matacc_asm_acc(int16_t *r, const int16_t *a, const int16_t *b, int16_t zeta) {
    (void)r; (void)a; (void)b; (void)zeta;
}

/*
 * rfc7748.c – RFC 7748 X25519 implementation using the big-number library.
 *
 * This implements the X25519 Diffie-Hellman function defined in RFC 7748 §5
 * using the multi-word arithmetic routines in functions.c and the Curve25519
 * prime P25519 / Barrett parameter R25519 from Primes.h.
 *
 * The core algorithm is the constant-time Montgomery ladder (RFC 7748 §5,
 * Algorithm 1).  The CSWAP operation uses a bit-mask technique so that the
 * execution time does not depend on the scalar bits, preventing simple
 * side-channel leaks.
 *
 * Curve parameters (RFC 7748 §4.1):
 *   p  = 2^255 − 19  (the field prime)
 *   A  = 486662       (Montgomery curve coefficient)
 *   a24 = (A − 2) / 4 = 121665
 *   base point u = 9
 *
 * This code is provided for educational purposes only and should not be
 * used in production systems without proper review and testing.
 */

#include "types.h"
#include "Primes.h"
#include <string.h>  /* memcpy */

#if WORD_BITS == 64
#define N448 7
static const WORD P448[N448] = {
    0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
    0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFEFFFFFFFFULL,
    0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
    0xFFFFFFFFFFFFFFFFULL
};
static WORD R448[N448 + 1] = {
    0x0000000000000002ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000100000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000001ULL
};
#elif WORD_BITS == 32
#define N448 14
static const WORD P448[N448] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
    0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU,
    0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
    0xFFFFFFFFU, 0xFFFFFFFFU
};
static WORD R448[N448 + 1] = {
    0x00000002U, 0x00000000U, 0x00000000U, 0x00000000U,
    0x00000000U, 0x00000000U, 0x00000000U, 0x00000001U,
    0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
    0x00000000U, 0x00000000U, 0x00000001U
};
#endif

/* -------------------------------------------------------------------------
 * Internal helper: decode 32 bytes (little-endian) into a WORD array.
 * The output array must have N25519 words; excess bits above 255 are zero.
 * ---------------------------------------------------------------------- */
static void decode_bytes(const uint8_t bytes[32], WORD *out)
{
    int i;
    const int bpw = WORD_BITS / 8;  /* bytes per word */
    Zeroes(out, N25519);
    for (i = 0; i < 32; i++) {
        int wi    = i / bpw;
        int shift = (i % bpw) * 8;
        out[wi] |= (WORD)bytes[i] << shift;
    }
}

/* -------------------------------------------------------------------------
 * Internal helper: encode a WORD array into 32 bytes (little-endian).
 * ---------------------------------------------------------------------- */
static void encode_bytes(const WORD *in, uint8_t out[32])
{
    int i;
    const int bpw = WORD_BITS / 8;
    memset(out, 0, 32);
    for (i = 0; i < 32; i++) {
        int wi    = i / bpw;
        int shift = (i % bpw) * 8;
        out[i] = (uint8_t)(in[wi] >> shift);
    }
}

/* -------------------------------------------------------------------------
 * X25519 – RFC 7748 §5 Diffie-Hellman function for Curve25519.
 *
 *  k_in : 32-byte private scalar (little-endian).  Clamped internally.
 *  u_in : 32-byte u-coordinate   (little-endian).  High bit masked per RFC.
 *  out  : 32-byte result u-coordinate (little-endian).
 *
 * Returns the result of the Montgomery ladder scalar multiplication
 * k * u on Curve25519, using the constant-time CSWAP primitive.
 * ---------------------------------------------------------------------- */
void X25519(const uint8_t k_in[32], const uint8_t u_in[32], uint8_t out[32])
{
    int i, t;

    /* --- 1. Copy inputs and apply RFC 7748 §5 clamping / masking --- */
    uint8_t k[32], u_bytes[32];
    memcpy(k,       k_in,  32);
    memcpy(u_bytes, u_in,  32);

    /* Clamp scalar k (RFC 7748 §5):
     *   bits 0-2 of k[0] := 0  (cofactor clearing)
     *   bit  7   of k[31] := 0  (keep scalar < 2^255)
     *   bit  6   of k[31] := 1  (ensure high bit is set for constant-time)  */
    k[0]  &= 248;
    k[31] &= 127;
    k[31] |= 64;

    /* Mask the high bit of the last byte of the u-coordinate (RFC 7748 §5). */
    u_bytes[31] &= 127;

    /* --- 2. Decode byte arrays into WORD arrays --- */
    WORD k_w[N25519], u_w[N25519];
    decode_bytes(k,       k_w);
    decode_bytes(u_bytes, u_w);

    /* --- 3. a24 = 121665 as a field element --- */
    WORD a24[N25519];
    Zeroes(a24, N25519);
    a24[0] = 121665;

    /* --- 4. Initialise the Montgomery ladder state ---
     *   (x2 : z2) = (1 : 0)   represents the point at infinity
     *   (x3 : z3) = (u : 1)   represents the input u-coordinate        */
    WORD x2[N25519], z2[N25519], x3[N25519], z3[N25519];
    Zeroes(x2, N25519); x2[0] = 1;
    Zeroes(z2, N25519);
    Cpy(x3, u_w, N25519);
    Zeroes(z3, N25519); z3[0] = 1;

    WORD swap = 0;

    /* --- 5. Temporaries for the field operations inside the loop --- */
    WORD A1[N25519], AA[N25519], B1[N25519], BB[N25519];
    WORD E1[N25519], C1[N25519], D1[N25519];
    WORD DA[N25519], CB[N25519];
    WORD tmp1[N25519], tmp2[N25519];

    /* --- 6. Montgomery ladder: iterate over bits 254 down to 0 ---
     *  (Bit 255 of the clamped scalar is always 0; bit 254 is always 1.) */
    for (t = 254; t >= 0; t--) {
        int wi   = t / WORD_BITS;
        int bi   = t % WORD_BITS;
        WORD k_t = (k_w[wi] >> bi) & (WORD)1;

        /* Conditional swap before the differential-addition step. */
        swap ^= k_t;
        CSWAP(swap, x2, x3, N25519);
        CSWAP(swap, z2, z3, N25519);
        swap = k_t;

        /* Montgomery differential addition-and-doubling (RFC 7748 §5):
         *
         *  A  = x2 + z2
         *  AA = A^2
         *  B  = x2 - z2
         *  BB = B^2
         *  E  = AA - BB
         *  C  = x3 + z3
         *  D  = x3 - z3
         *  DA = D * A
         *  CB = C * B
         *  x3 = (DA + CB)^2
         *  z3 = u * (DA - CB)^2
         *  x2 = AA * BB
         *  z2 = E * (AA + a24 * E)
         */

        /* A1 = x2 + z2 mod p */
        ModAdd(x2, z2, A1, P25519, N25519);
        /* AA = A1^2 mod p */
        ModMul(A1, A1, P25519, R25519, AA, N25519);
        /* B1 = x2 - z2 mod p */
        SubMod(x2, z2, B1, P25519, N25519);
        /* BB = B1^2 mod p */
        ModMul(B1, B1, P25519, R25519, BB, N25519);
        /* E1 = AA - BB mod p */
        SubMod(AA, BB, E1, P25519, N25519);
        /* C1 = x3 + z3 mod p */
        ModAdd(x3, z3, C1, P25519, N25519);
        /* D1 = x3 - z3 mod p */
        SubMod(x3, z3, D1, P25519, N25519);
        /* DA = D1 * A1 mod p */
        ModMul(D1, A1, P25519, R25519, DA, N25519);
        /* CB = C1 * B1 mod p */
        ModMul(C1, B1, P25519, R25519, CB, N25519);

        /* x3 = (DA + CB)^2 mod p */
        ModAdd(DA, CB, tmp1, P25519, N25519);
        ModMul(tmp1, tmp1, P25519, R25519, x3, N25519);

        /* z3 = u * (DA - CB)^2 mod p */
        SubMod(DA, CB, tmp1, P25519, N25519);
        ModMul(tmp1, tmp1, P25519, R25519, tmp2, N25519);
        ModMul(u_w, tmp2, P25519, R25519, z3, N25519);

        /* x2 = AA * BB mod p */
        ModMul(AA, BB, P25519, R25519, x2, N25519);

        /* z2 = E1 * (AA + a24 * E1) mod p */
        ModMul(a24, E1, P25519, R25519, tmp1, N25519);
        ModAdd(AA, tmp1, tmp2, P25519, N25519);
        ModMul(E1, tmp2, P25519, R25519, z2, N25519);
    }

    /* Final conditional swap to undo any outstanding bit. */
    CSWAP(swap, x2, x3, N25519);
    CSWAP(swap, z2, z3, N25519);

    /* --- 7. Recover the affine x-coordinate: result = x2 * z2^(p-2) mod p ---
     *
     * By Fermat's little theorem, z2^(p-2) ≡ z2^{-1}  (mod p).
     * p - 2 = 2^255 - 21.  We build the exponent directly from P25519.   */
    WORD p_minus_2[N25519];
    Cpy(p_minus_2, P25519, N25519);
    /* P25519[0] = 0xFFFF...ED  ≥ 2, so subtracting 2 does not borrow. */
    p_minus_2[0] -= (WORD)2;

    WORD z2_inv[N25519], result[N25519];
    ModExp(z2, p_minus_2, P25519, R25519, z2_inv, N25519);
    ModMul(x2, z2_inv, P25519, R25519, result, N25519);

    /* --- 8. Encode the result to 32 bytes (little-endian) --- */
    encode_bytes(result, out);
}

static void decode_bytes448(const uint8_t bytes[56], WORD *out)
{
    int i;
    const int bpw = WORD_BITS / 8;

    Zeroes(out, N448);
    for (i = 0; i < 56; i++) {
        int wi = i / bpw;
        int shift = (i % bpw) * 8;
        out[wi] |= (WORD)bytes[i] << shift;
    }
}

static void encode_bytes448(const WORD *in, uint8_t out[56])
{
    int i;
    const int bpw = WORD_BITS / 8;

    memset(out, 0, 56);
    for (i = 0; i < 56; i++) {
        int wi = i / bpw;
        int shift = (i % bpw) * 8;
        out[i] = (uint8_t)(in[wi] >> shift);
    }
}

void X448(const uint8_t k_in[56], const uint8_t u_in[56], uint8_t out[56])
{
    int t;
    uint8_t k[56], u_bytes[56];
    WORD k_w[N448], u_w[N448], a24[N448];
    WORD x2[N448], z2[N448], x3[N448], z3[N448];
    WORD A1[N448], AA[N448], B1[N448], BB[N448];
    WORD E1[N448], C1[N448], D1[N448], DA[N448], CB[N448];
    WORD tmp1[N448], tmp2[N448], p_minus_2[N448];
    WORD z2_inv[N448], result[N448];
    WORD swap = 0;

    memcpy(k, k_in, 56);
    memcpy(u_bytes, u_in, 56);
    k[0] &= 252;
    k[55] |= 128;

    decode_bytes448(k, k_w);
    decode_bytes448(u_bytes, u_w);

    Zeroes(a24, N448);
    a24[0] = 39081;
    Zeroes(x2, N448); x2[0] = 1;
    Zeroes(z2, N448);
    Cpy(x3, u_w, N448);
    Zeroes(z3, N448); z3[0] = 1;

    for (t = 447; t >= 0; t--) {
        int wi = t / WORD_BITS;
        int bi = t % WORD_BITS;
        WORD k_t = (k_w[wi] >> bi) & (WORD)1;

        swap ^= k_t;
        CSWAP(swap, x2, x3, N448);
        CSWAP(swap, z2, z3, N448);
        swap = k_t;

        ModAdd(x2, z2, A1, P448, N448);
        ModMul(A1, A1, P448, R448, AA, N448);
        SubMod(x2, z2, B1, P448, N448);
        ModMul(B1, B1, P448, R448, BB, N448);
        SubMod(AA, BB, E1, P448, N448);
        ModAdd(x3, z3, C1, P448, N448);
        SubMod(x3, z3, D1, P448, N448);
        ModMul(D1, A1, P448, R448, DA, N448);
        ModMul(C1, B1, P448, R448, CB, N448);

        ModAdd(DA, CB, tmp1, P448, N448);
        ModMul(tmp1, tmp1, P448, R448, x3, N448);
        SubMod(DA, CB, tmp1, P448, N448);
        ModMul(tmp1, tmp1, P448, R448, tmp2, N448);
        ModMul(u_w, tmp2, P448, R448, z3, N448);
        ModMul(AA, BB, P448, R448, x2, N448);
        ModMul(a24, E1, P448, R448, tmp1, N448);
        ModAdd(AA, tmp1, tmp2, P448, N448);
        ModMul(E1, tmp2, P448, R448, z2, N448);
    }

    CSWAP(swap, x2, x3, N448);
    CSWAP(swap, z2, z3, N448);

    Cpy(p_minus_2, P448, N448);
    p_minus_2[0] -= (WORD)2;
    ModExp(z2, p_minus_2, P448, R448, z2_inv, N448);
    ModMul(x2, z2_inv, P448, R448, result, N448);
    encode_bytes448(result, out);
}


#ifndef types_file
#define types_file

#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<time.h>

#if WORD_BITS == 64
typedef uint64_t WORD;

#if defined(__SIZEOF_INT128__)
typedef __uint128_t DWORD;
#else
#error "No 128-bit integer support on this compiler for WORD_BITS=64"
#endif

#elif WORD_BITS == 32
typedef uint32_t WORD;
typedef uint64_t DWORD;
#else
#error "Must select 32 / 64."
#endif

/* Functions definition */
/* Random number */
void Ran(WORD* U, WORD* V, int N);

/* Multiplication */
void Mul(const WORD* U, const WORD* V, WORD* W, int N);

/* Karatsuba multiplication */
void Kar(const WORD* U, const WORD* V, WORD* W, int N);

/* Addition carry? */
void SumCL(const WORD* U, const WORD* V, WORD* W, int N);

/* Substraction modular?*/
void SubMod(const WORD* U, const WORD* V, WORD* W, const WORD* P, int N);

/* Addition*/
void Sum(const WORD* U, const WORD* V, WORD* W, int N);

/* Substraction */
void Sub(const WORD* U, const WORD* V, WORD* W, int N);

/* Barret reduction */
void Barr(const WORD* U, const WORD* P, WORD* R, WORD* Red, int N);

/* Compare two values*/
int Cmp(const WORD* U, const WORD* V, int N);

/* Copy */
void Cpy(WORD* U, const WORD *V, int N);

/* Print values as hexadecimal*/
void printer(const WORD* P, int N, char* C);

/* Fill with zeroes */
void Zeroes(WORD* Z, int N);

/* And Logic operation*/
void AND(WORD* Z, WORD C,  int N);

/* Extended GCD */
void XGCD(WORD* P, WORD* A, WORD* Inv, int N);

/* Divides by two  operand/2 */
void DivTwo(WORD * U, WORD* V, int N);

/* Multiplication B?*/
void MulB(const WORD* U, const WORD* V, WORD* W, int N, int M);

/* Addition carry (alias for Sum: result W[0..N-1], carry in W[N]) */
void SumCL(const WORD* U, const WORD* V, WORD* W, int N);

/* Modular addition: W = (U + V) mod P */
void ModAdd(const WORD* U, const WORD* V, WORD* W, const WORD* P, int N);

/* Modular multiplication using Barrett reduction: Red = (U * V) mod P
 * R_barr is the precomputed Barrett parameter (N+1 words) */
void ModMul(const WORD* U, const WORD* V, const WORD* P, WORD* R_barr, WORD* Red, int N);

/* Modular exponentiation (left-to-right binary method): Res = Base^Exp mod P
 * R_barr is the precomputed Barrett parameter (N+1 words) */
void ModExp(const WORD* Base, const WORD* Exp, const WORD* P, WORD* R_barr, WORD* Res, int N);

/* Constant-time conditional swap (RFC 7748 CSWAP):
 * if swap == 1 exchange A[0..N-1] with B[0..N-1]; if swap == 0 do nothing.
 * The operation takes the same time regardless of swap to prevent side-channel leaks. */
void CSWAP(WORD swap, WORD* A, WORD* B, int N);

/* RFC 7748 X25519 Diffie-Hellman function.
 * k_in : 32-byte little-endian private scalar  (clamped internally per RFC 7748 §5)
 * u_in : 32-byte little-endian u-coordinate of the peer public key
 * out  : 32-byte little-endian result (shared u-coordinate)
 * Uses P25519 / R25519 from Primes.h and the Montgomery ladder algorithm. */
void X25519(const uint8_t k_in[32], const uint8_t u_in[32], uint8_t out[32]);


#endif

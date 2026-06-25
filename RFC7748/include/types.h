#ifndef types_file
#define types_file

#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<time.h>

#ifndef WORD_BITS
	#error "Define WORD_BITS=32 or WORD_BITS=64."
#endif

#if WORD_BITS == 64
/* Definition used on x64 architectures */
typedef uint64_t WORD;
typedef __uint128_t DWORD;

#elif WORD_BITS == 32
/* Definition used on x86 architectures */
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


#endif

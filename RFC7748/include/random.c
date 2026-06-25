#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<time.h>

#ifndef WORD_BITS
	#error "Define WORD_BITS=32 or WORD_BITS=64."
#endif

#if WORD_BITS == 64
	typedef uint64_t WORD;

#elif WORD_BITS == 32
	typedef uint32_t WORD;

#else
	#error "Must select 32 / 64."
#endif

void Ran(WORD* U, WORD* V, int N);

void Ran(WORD* U, WORD* V, int N){
	int i;
	for (i = 0; i < N; i++)
	{
		U[i] = rand() & 0x0000FFFFFFFFFFFF;
		U[i] = (U[i] << 32) | rand();
		V[i] = rand() & 0x00FFFFFFFFFFFFFF;
		V[i] = (V[i] << ((WORD)32)) | (WORD)rand();
	}
}

void main(int argc, char const *argv[])
{
	time_t t;
	srand((unsigned)time(&t));
	int N = 80, i;
	WORD U[N];
	WORD V[N];

	printf("Generando palabra aleatoria\n");
	Ran(U, V, N);
	printf("Palabras generadas: \n");
	printf("U := [ ");

	#if WORD_BITS == 64
	/* 64-bit version*/
	for (i = 0; i < N-1; ++i)
	{
		printf("0x%.16jX, ",U[i] );
	}
	printf("0x%.16jX ]; \n",U[i] );
	printf("V := [ ");
	for (i = 0; i < N-1; ++i)
	{
		printf("0x%.16jX, ",V[i] );
	}
	printf("0x%.16jX ]; \n",V[i] );

	#elif WORD_BITS == 32
	/* 32-bit version */
	for (i = 0; i < N-1; ++i)
	{
		printf("0x%.8jX, ",U[i] );
	}
	printf("0x%.8jX ]; \n",U[i] );
	printf("V := [ ");
	for (i = 0; i < N-1; ++i)
	{
		printf("0x%.8jX, ",V[i] );
	}
	printf("0x%.8jX ]; \n",V[i] );

	#else
		#error "Must select 32 / 64"
 	#endif
}

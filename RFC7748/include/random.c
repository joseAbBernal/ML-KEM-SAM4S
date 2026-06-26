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
#if WORD_BITS == 64
    for (i = 0; i < N; i++)
    {
        U[i] = ((uint64_t)rand() << 32) | rand();
        V[i] = ((uint64_t)rand() << 32) | rand();
    }
#elif WORD_BITS == 32
    for (i = 0; i < N; i++)
    {
        U[i] = rand();
        V[i] = rand();
    }
#endif
}

int main(int argc, char const *argv[])
{
	time_t t;
	srand((unsigned)time(&t));
	int N = 80, i;
	WORD* U = (WORD*)malloc(N * sizeof(WORD));
    WORD* V = (WORD*)malloc(N * sizeof(WORD));

	if (U == NULL || V == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        return 1;
    }

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

	free(U);
	free(V);

	return 0;
}

#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<time.h>

typedef uint64_t WORD;

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

}

#include <stdio.h>
#include <inttypes.h>

void main(int argc, char const *argv[])
{
 // div 2
	int i, N = 16;
	uint64_t A[16] = {0xDD50FC3D66B43908, 0x84D153FA93E6F634, 0xECC4A245EC14D4AF, 0x498427FDD9BF4C3F, 
0xEFEB46D699FE7B7B, 0x48B838221BBEAA11, 0x7E3D03BE85D5570D, 0xAC4402B1526081C4, 
0x9AF924AD70B9B746, 0xD98343017830DD41, 0x3BC8D4E3A36355F3, 0xDACFFF91CE4EEF30, 
0x9178CAC30E27EAC7, 0x1CFB1A011821C5AF, 0xB146E085231EF5D6, 0x360137CBFC74EBBC};

	uint64_t lsb, aux[16]= {0};

	//printf("0x%.16jX\n",lsb );
	for (i = 0; i < N-1; i++)
	{
		lsb = A[i+1] & 0x1;
		A[i] = (A[i] >> 1) ^ (lsb << 63);
	}
	A[N-1] = A[N-1] >> 1;
/*	lsb = A[N-1] & 0x1;
	aux[N-1] = A[N-1] >> 1;
	printf("0x%.16jX\n",lsb );
	for (i = N-2; i >= 0; i--)//for (i = 1; i < N; i++)
	{
		aux[i] = (A[i] >> 1) ^ (lsb << 63);
		lsb = A[i] & 0x1;
		printf("0x%.16jX\n",lsb );
	}	*/
/// div 2
	for (i = 0; i < N; i++)//for (i = N-1; i >= 0; i--)
	{
		printf("0x%.16jx, ",A[i]);
	}
	printf("\n");


}
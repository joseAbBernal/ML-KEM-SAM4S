#include<stdio.h>
#include<inttypes.h>

#ifndef WORD_BITS
	#error "Define WORD_BITS=32 or WORD_BITS=64"
#endif

#if WORD_BITS == 64
/* 64-bit version*/
	uint64_t U[36] = {0x7B6A839C2C66E369, 0x7AD45467EF335661, 0x17E6FB38DFB5169C, 0xA947FFF952B17639, 
				  0x96028AE4D4A05AEE, 0xAAFACD07382FD009, 0x4FA53723DAF00A17, 0xDCD80417658E4C7A, 
				  0x74BA18B58FFFDE55, 0xE25E0FD2C69D6479, 0xF3595E1E96C4B89F, 0x7E0EF695C63114E7, 
				  0xC99664D161EA70F8, 0x42CC645E5C633D05, 0x8481B31E35036ED8, 0x24926EF88F947410, 
				  0x2C198D44C08FA4E9, 0x5959E81C88A62960, 0xBAB2E6A9EB6596FD, 0x9D084D32C8A8FE2, 
				  0x613C5E87DD1E0D89, 0x39A6FC7ECED7EA4C, 0x1E6D4CA178C3DEC1, 0x902E74FDA90CF96C, 
				  0xAEA2E1595231926, 0x2C2536335724E723, 0x6FBB6BFA03F7E6AF, 0x4A7849308D9F44DD, 
				  0xFCE4CCCCFEF39D27, 0xF8F63D5D61409E1, 0xC64C0415B64CC219, 0x22F46DEB167267F4, 
				  0x941B487F07FC7D23, 0xBE31B92DF95EF5C0, 0xE50B91093B5B013B, 0x947C06AF03B9A98A };
	int N = 36;	//number of words 36 -> 64-bit; 72 -> 32-bit
	int word = 64;	//Word size in bits 64 -> 64-bit; 32 -> 32-bit

#elif WORD_BITS == 32
		/* 32-bit words version */
	uint32_t U[72] = {0x2C66E369, 0x7B6A839C, 0xEF335661, 0x7AD45467, 0xDFB5169C, 0x17E6FB38, 
					  0x52B17639, 0xA947FFF9, 0xD4A05AEE, 0x96028AE4, 0x382FD009, 0xAAFACD07, 
					  0xDAF00A17, 0x4FA53723, 0x658E4C7A, 0xDCD80417, 0x8FFFDE55, 0x74BA18B5, 
					  0xC69D6479, 0xE25E0FD2, 0x96C4B89F, 0xF3595E1E, 0xC63114E7, 0x7E0EF695, 
					  0x61EA70F8, 0xC99664D1, 0x5C633D05, 0x42CC645E, 0x35036ED8, 0x8481B31E, 
					  0x8F947410, 0x24926EF8, 0xC08FA4E9, 0x2C198D44, 0x88A62960, 0x5959E81C, 
					  0xEB6596FD, 0xBAB2E6A9, 0x2C8A8FE2, 0x9D084D3, 0xDD1E0D89, 0x613C5E87, 
					  0xCED7EA4C, 0x39A6FC7E, 0x78C3DEC1, 0x1E6D4CA1, 0xA90CF96C, 0x902E74FD, 
					  0x95231926, 0xAEA2E15, 0x5724E723, 0x2C253633, 0x3F7E6AF, 0x6FBB6BFA, 
					  0x8D9F44DD, 0x4A784930, 0xFEF39D27, 0xFCE4CCCC, 0xD61409E1, 0xF8F63D5, 
					  0xB64CC219, 0xC64C0415, 0x167267F4, 0x22F46DEB, 0x7FC7D23, 0x941B487F, 
					  0xF95EF5C0, 0xBE31B92D, 0x3B5B013B, 0xE50B9109, 0x3B9A98A, 0x947C06AF };
	int N = 72;	//number of words 36 -> 64-bit; 72 -> 32-bit
	int word = 32;	//Word size in bits 64 -> 64-bit; 32 -> 32-bit
#else
	#error "Must select 32 / 64"
#endif

int main(int argc, char const *argv[])
{
	int cor = 4;   		//Number of bits per word 64 -> 4; 32 -> 4
	for (int i = 0; i < N; ++i)
	{
		#ifdef WORD_BITS
			#if WORD_BITS == 64
				/* Print function for 64-bit words */
				printf("Palabra U[%i]\n", i );
				for (int j = 0; j <= (word/cor); j++)
				{
					uint64_t c = U[i] >> (j*4) ;
					printf("0x%.16jX \n",c );
				}
			#elif WORD_BITS == 32
				/* Print function for 32-bit words */
				printf("Palabra U[%i]\n", i );
				for (int j = 0; j <= (word/cor); j++)
				{
					uint32_t c = U[i] >> (j*4) ;
					printf("0x%.8jX \n",c );
				}
			#endif
		#endif
	}

	return 0;
}

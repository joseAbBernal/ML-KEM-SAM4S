#include "Bench.h"
#include "types.h"
#include "Primes.h"
#include <stdio.h>


#define BENCH_LOOPS           100000       // Number of iterations per bench

void bench(int W)
{
    int n;
    unsigned long long cycles, cycles1, cycles2;
    WORD A[W], B[W], C[W+1];
    WORD P[W], R[W], Red[W], Inv[W];
    WORD Ab[2*W];
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking Addition: \n\n"); 
    // Addition
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        Ran(A, B, W);
        cycles1 = cpucycles(); 
        Sum(A, B, C, W);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("Addition runs in .......................................... %7lldu ", cycles/BENCH_LOOPS); print_unit;
    printf("\n");
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking Subtraction: \n\n");
    // Subtraction 
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        Ran(A, B, W);
        cycles1 = cpucycles(); 
        Sub(A, B, C, W);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("Subtraction runs in ....................................... %7lldu ", cycles/BENCH_LOOPS); print_unit;
    printf("\n");
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking Schoolbook multiplication: \n\n");
    // schoolbook multiplication
    cycles = 0;
    C[2*W];
    for (n=0; n<BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles(); 
        Mul(A, B, C, W);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("Schoolbook multiplication runs in .................................... %7lldu ", cycles/BENCH_LOOPS); print_unit;
    printf("\n");
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking Karatsuba: \n\n");
    // Karatsuba multiplication
    cycles = 0;
    for (n=0; n<BENCH_LOOPS; n++)
    {
        cycles1 = cpucycles(); 
        Kar(A, B, C, W);
        cycles2 = cpucycles();
        cycles = cycles+(cycles2-cycles1);
    }
    printf("Karatsuba runs in ......................................... %7lldu ", cycles/BENCH_LOOPS); print_unit;
    printf("\n");
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking Barret reduction: \n\n");
    // Barret reduction
    cycles = 0;
        switch(W){
        case 4 :
            for (n=0; n<BENCH_LOOPS; n++)
            {
                Ran(Ab, Ab, 2*W);
                cycles1 = cpucycles();
                Barr(Ab, P_4, R_4, Red, W);
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 8 :
            for (n=0; n<BENCH_LOOPS; n++)
            {
                Ran(Ab, Ab, 2*W);
                cycles1 = cpucycles();
                Barr(Ab, P_8, R_8, Red, W);
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 16 :
            for (n=0; n<BENCH_LOOPS; n++)
            {
                Ran(Ab, Ab, 2*W);
                cycles1 = cpucycles();
                Barr(Ab, P_16, R_16, Red, W);
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 32 :
            for (n=0; n<BENCH_LOOPS; n++)
            {
                Ran(Ab, Ab, 2*W);
                cycles1 = cpucycles();
                Barr(Ab, P_32, R_32, Red, W);
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 64 :
            for (n=0; n<BENCH_LOOPS; n++)
            {
                Ran(Ab, Ab, 2*W);
                cycles1 = cpucycles();
                Barr(Ab, P_64, R_64, Red, W);
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;

        default :
         printf("Not prime available to bench\n" );
         exit(0);
    }
    printf("Barret reduction for %d Words runs in ........................ %7lldu ", W, cycles/BENCH_LOOPS); print_unit;
    printf("\n");
    printf("\n--------------------------------------------------------------------------------------------------------\n\n"); 
    printf("Benchmarking extended binary GCD: \n\n");
    // Extended GCD
    cycles = 0;
    switch(W){
        case 4 :
            for (n=0; n<BENCH_LOOPS; n++)
            {   
                Ran(A,A,W);
                cycles1 = cpucycles(); 
                XGCD(P_4, A, Inv, W); //prime p, number < p, inverse, size
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 8 :
            for (n=0; n<BENCH_LOOPS; n++)
            {   
                Ran(A,A,W);
                cycles1 = cpucycles(); 
                XGCD(P_8, A, Inv, W); //prime p, number < p, inverse, size
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 16 :
            for (n=0; n<BENCH_LOOPS; n++)
            {   
                Ran(A,A,W);
                cycles1 = cpucycles(); 
                XGCD(P_16, A, Inv, W); //prime p, number < p, inverse, size
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 32 :
            for (n=0; n<BENCH_LOOPS; n++)
            {   
                Ran(A,A,W);
                cycles1 = cpucycles(); 
                XGCD(P_32, A, Inv, W); //prime p, number < p, inverse, size
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;
        case 64 :
            for (n=0; n<BENCH_LOOPS; n++)
            {   
                Ran(A,A,W);
                cycles1 = cpucycles(); 
                XGCD(P_64, A, Inv, W); //prime p, number < p, inverse, size
                cycles2 = cpucycles();
                cycles = cycles+(cycles2-cycles1);
            }
        break;

        default :
         printf("Not prime available to bench\n" );
         exit(0);
    }
    printf("Extended binary GCD for %d Words runs in ........................ %7lldu ", W, cycles/BENCH_LOOPS); print_unit;
    printf("\n");
}


int main(int argc, char const *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <W>\n", argv[0]);
        return 1;
    }

    int W = atoi(argv[1]);  
    bench(W);
    return 0;
}



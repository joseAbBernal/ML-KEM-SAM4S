#include "types.h"
#include "Primes.h"

#if WORD_BITS == 64
	#define BASE "2^64"
#elif WORD_BITS == 32
	#define BASE "2^32"
#else
	#error "Must select 32 / 64."
#endif

void testKar(int N){
	int i;
	WORD U[N];						//different size words
	WORD V[N];						//different size words
	WORD W[2*N];					//array of words with the result
	Ran(U, V, N);					//Random Word Generator 
	//	Words generated with Ran function	///
	printer(U, N, "U");
	printer(V, N, "V");
	//	End print of Words generated with Ran function	//
	Kar(U, V, W, N);				//Karatsuba Multiplication U*V
    printer(W, 2*N, "C");			//print result C = U*V with karatsuba method
	// Magma program to test multiply function//
	printf("u := VectorToInteger(U);\n");
	printf("v := VectorToInteger(V);\n");
	printf("d := u*v;\n");
	printf("D := Intseq(d, %s);\n", MAGMA_BASE_STR);
	printf("if D ne C then \n");
	printf("\t\"U:=\",U:Hex;\n");
	printf("\t\"V:=\",V:Hex;\n");
	printf("\t\"D:=\",D:Hex;\n");
	printf("\t\"C:=\",C:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

void testMul(int N){
	int i;
	WORD U[N];						//different size words
	WORD V[N];						//different size words
	WORD W[2*N];					//array of words with the result
	Ran(U, V, N);					//Random Word Generator 
	//	Words generated with Ran function	///
	printer(U, N, "U");
	printer(V, N, "V");
	//	End print of Words generated with Ran function	//
	Mul(U, V, W, N);				//Multiply U*V
    printer(W, 2*N, "C");				//print the result of U*V
	// Magma program to test multiply function//
	printf("u := VectorToInteger(U);\n");
	printf("v := VectorToInteger(V);\n");
	printf("d := u*v;\n");
	printf("D := Intseq(d, BASE);\n");
	printf("if D ne C then \n");
	printf("\t\"U:=\",U:Hex;\n");
	printf("\t\"V:=\",V:Hex;\n");
	printf("\t\"D:=\",D:Hex;\n");
	printf("\t\"C:=\",C:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

void testSum(int N){
	
	int i;
	WORD U[N];				//different size words
	WORD V[N];				//different size words
	WORD W[N+1];			//array of words with the result
	Ran(U, V, N);			//Function to generate random words
	//	print words generated U and V	 ///
	printer(U, N, "U");
	printer(V, N, "V");
	Sum(U, V, W, N);		//made the addition of U and V;
    printer(W, N, "C");	//print the result of the addition
	// Magma program to test the addition//
	printf("u := VectorToInteger(U);\n");
	printf("v := VectorToInteger(V);\n");
	printf("d := u+v;\n");
	printf("D := Intseq(d, BASE);\n");
	printf("if D ne C then \n");
	printf("\t\"U:=\",U:Hex;\n");
	printf("\t\"V:=\",V:Hex;\n");
	printf("\t\"D:=\",D:Hex;\n");
	printf("\t\"C:=\",C:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

void testSub(int N){
	int i;
	WORD U[N];						//different size words
	WORD V[N];						//different size words
	WORD W[N+1];					//array of words with the result
	Ran(U, V, N);					//Random word generator for U and V
	// printing	Words Generated	///
	printer(U, N, "U");
	printer(V, N, "V");
	Sub(U, V, W, N);				//made the substraction
    printer(W, N, "C");				//print the result of U-V
	// Magma program to test the addition//
	printf("u := VectorToInteger(U);\n");
	printf("v := VectorToInteger(V);\n");
	printf("d := Abs(u-v);\n");
	printf("D := Intseq(d, BASE);\n");
	printf("if D ne C then \n");
	printf("\t\"U:=\",U:Hex;\n");
	printf("\t\"V:=\",V:Hex;\n");
	printf("\t\"D:=\",D:Hex;\n");
	printf("\t\"C:=\",C:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

void testCmp(int N){
	int i;
	WORD U[N], V[N];
	i = Cmp(U, V, N);
	if (i == 0){
		printf("U y V son iguales\n");
	}else if (i == -1){
		printf("V es mayor que U\n");
	}else{
		printf("U es mayor que V\n");
	}
}

void testCPY(int N){
	int i;
	WORD U[N], V[N];
	i = Cmp(U, V, N);

	if (i == 0){
		printf("U y V son iguales\n");
	}else if (i == -1){
		printf("V es mayor que U\n");
		Cpy(U, V, N);
		printf("Copiados V -> U : Y : U -> V\n");
	}else{
		printf("U es mayor que V\n");
	}
	printer(U, N, "U");
	printer(V, N, "V");
	printf("Comprobando por segunda vez\n");
	i = Cmp(U, V, N);
	if (i == 0){
		printf("U y V son iguales\n");
	}else if (i == -1){
		printf("V es mayor que U\n");
		Cpy(U, V, N);
	}else{
		printf("U es mayor que V\n");
	}
}

void testRan(int N){
	int i;
	WORD U[N];
	WORD V[N];
	printf("Generating random numbers\n");
	Ran(U, V, N);
	printf("Numbers generated: \n");
	printer(U, N, "U");
	printer(V, N, "V");
}

void testDiv(int N){
	int i;
	WORD U[N];
	WORD V[N];
	Zeroes(V, N);
	Ran(U, U, N);
	printer(U, N, "U");
	DivTwo(U, V, N);
	printer(V, N, "V");
	// Magma program to test the addition//
	printf("\n");
	printf("if (Intseq(ShiftRight(Seqint(U, BASE),1), BASE) ne V) then \n");
	printf("\t\"U:=\",U:Hex;\n");
	printf("\t\"V:=\",V:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

void testXGCD(int N){
	WORD Inv[N], A[N];
	Ran(A, A, N);
	switch(N){
		case 4 :
			printer(P_4, N, "P");
			printer(A, N, "A");
			XGCD(P_4, A, Inv, N);
			printer(Inv, N, "Inv");
		break;
		case 8 :
			printer(P_8, N, "P");
			printer(A, N, "A");
			XGCD(P_8, A, Inv, N);
			printer(Inv, N, "Inv");
		break;
		case 16 :
			printer(P_16, N, "P");
			printer(A, N, "A");
			XGCD(P_16, A, Inv, N);
			printer(Inv, N, "Inv");
		break;
		case 32 :
			printer(P_32, N, "P");
			printer(A, N, "A");
			XGCD(P_32, A, Inv, N);
			printer(Inv, N, "Inv");
		break;
		case 64 :
			printer(P_64, N, "P");
			printer(A, N, "A");
			XGCD(P_64, A, Inv, N);
			printer(Inv, N, "Inv");
		break;
		case 128 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 256 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 512 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 1024 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;

		default :
         printf("Not prime available to test\n" );
	}
	printf("p := VectorToInteger(P);\n");
	printf("a := VectorToInteger(A);\n");
	printf("d := XHGC(p, a);\n");
	printf("D := Intseq(d, BASE);\n");
	printf("if D ne Inv then \n");
	printf("\t\"P:=\",P:Hex;\n");
	printf("\t\"A:=\",A:Hex;\n");
	printf("\t\"D:=\",D:Hex;\n");
	printf("\t\"Inv:=\",Inv:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
	
}

void testBarret(int N){
	WORD A[2*N], Red[N];
	Ran(A, A, 2*N);
	switch(N){
		case 4 :
			printer(P_4, N, "P");
			printer(R_4, N+1, "R");
			printer(A, 2*N, "A");
			Barr(A, P_4, R_4, Red, N);
			printer(Red, N, "Red");
		break;
		case 8 :
			printer(P_8, N, "P");
			printer(R_8, N+1, "R");
			printer(A, 2*N, "A");
			Barr(A, P_8, R_8, Red, N);
			printer(Red, N, "Red");
		break;
		case 16 :
			printer(P_16, N, "P");
			printer(R_16, N+1, "R");
			printer(A, 2*N, "A");
			Barr(A, P_16, R_16, Red, N);
			printer(Red, N, "Red");
		break;
		case 32 :
			printer(P_32, N, "P");
			printer(R_32, N+1, "R");
			printer(A, 2*N, "A");
			Barr(A, P_32, R_32, Red, N);
			printer(Red, N, "Red");
		break;
		case 64 :
			printer(P_64, N, "P");
			printer(R_64, N+1, "R");
			printer(A, 2*N, "A");
			Barr(A, P_64, R_64, Red, N);
			printer(Red, N, "Red");
		break;
		case 128 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 256 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 512 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;
		case 1024 :
			printf("\"Prime not available\";\n");
			printf("exit;\n");
			exit(0);
		break;

		default :
         printf("\"Not prime available to test\";\n" );
         printf("exit;\n");
	}
	printf("p := VectorToInteger(P);\n");
	printf("a := VectorToInteger(A);\n");
	printf("r := VectorToInteger(R);\n");
	printf("N := %d;\n",N);
	printf("rs := Barret(a,p,r, N);\n");
	printf("rr := Intseq(rs,BASE);\n");
	printf("if rr ne Red then \n");
	printf("\t\"P:=\",P:Hex;\n");
	printf("\t\"A:=\",A:Hex;\n");
	printf("\t\"Mu:=\",R:Hex;\n");	//it's R
	printf("\t\"rr:=\",rr:Hex;\n");
	printf("\t\"Red:=\",Red:Hex;\n");
	printf("\texit;\n");
	printf("end if;\n");
	// end of Magma program
}

int main(int argc, char const *argv[])
{
	if (argc < 3) {
        fprintf(stderr, "Uso: %s <sel> <N>\n", argv[0]);
        return 1;
    }

	time_t t;									//this must be here always
	srand((unsigned)time(&t));					//and this too
	int ctr = 0, sel = atoi(argv[1]), N = atoi(argv[2]);
	printf("load \"multiplica.magma\";\n");
	switch(sel) {
      case 1 :
      	 if (ctr == 0)
      	 {
      	 	printf("\"testing Addition\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing add function
		 {
			testSum(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
      case 2 :
      	if (ctr == 0)
      	 {
      	 	printf("\"testing Substraction\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing substraction function
		 {
			testSub(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
      case 3 :
         if (ctr == 0)
      	 {
      	 	printf("\"testing schoolbook multiplication\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing school book multiplication function
		 {
			testMul(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
      case 4 :
         if (ctr == 0)
      	 {
      	 	printf("\"testing karatsuba method\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing Karatsuba function
		 {
			testKar(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
      case 5 :
         if (ctr == 0)
      	 {
      	 	printf("\"testing Barret\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing Barret function
		 {
			testBarret(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
       case 6:
       	 if (ctr == 0)
      	 {
      	 	printf("\"testing XGCD\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing XGCD function
		 {
			testXGCD(N);
			printf("\n");
		 }
		 ctr = 0;
         break;
       case 7:
         if (ctr == 0)
      	 {
      	 	printf("\"testing Addition\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing add function
		 {
			testSum(N);
			printf("\n");
		 }
         if (ctr == 1)
      	 {
      	 	printf("\"testing Substraction\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing substraction function
		 {
			testSub(N);
			printf("\n");
		 }
         if (ctr == 2)
      	 {
      	 	printf("\"testing schoolbook multiplication\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing school book multiplication function
		 {
			testMul(N);
			printf("\n");
		 }
         if (ctr == 3)
      	 {
      	 	printf("\"testing karatsuba method\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing Karatsuba function
		 {
			testKar(N);
			printf("\n");
		 }
         if (ctr == 4)
      	 {
      	 	printf("\"testing Barret\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing Barret function
		 {
			testBarret(N);
			printf("\n");
		 }
       	 if (ctr == 5)
      	 {
      	 	printf("\"testing XGCD\";\n");
      	 	ctr += 1;
      	 }
         for (int i = 0; i < 10000; i++)			//testing XGCD function
		 {
			testXGCD(N);
			printf("\n");
		 }
         break;
      default :
         printf("Invalid test\n" );
   }
   printf("exit;\n");
   return 0;
}
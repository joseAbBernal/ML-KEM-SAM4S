#include "types.h"

void printer(const WORD* P, int N, char* C){
	int i;
	printf("%s := [", C);
	for (i = 0; i < N - 1; i++)
	{
		#if WORD_BITS == 64
			printf("0x%016" PRIx64 ", ", (uint64_t)P[i]);
		#elif WORD_BITS == 32
			printf("0x%08" PRIx32 ", ", (uint32_t)P[i]);
		#else
			#error "Must select 32 / 64."
		#endif
	}
	#if WORD_BITS == 64
		printf("0x%016" PRIx64 "];\n", (uint64_t)P[i]);
	#elif WORD_BITS == 32
		printf("0x%08" PRIx32 "];\n", (uint32_t)P[i]);
	#else
		#error "Must select 32 / 64."
	#endif
	printf("\n");
}

void Zeroes(WORD* Z, int N){
	int i;
	for (i = 0; i < N; i++)
	{
		Z[i] = 0;
	}
}

void AND(WORD* Z, WORD C,  int N){
	int i;
	for (i = 0; i < N; i++)
	{
		Z[i] = (Z[i] & C);
	}
}

void Kar(const WORD* U, const WORD* V, WORD* W, int N){
	int i, mid = N/2;
	WORD Us[mid+1], Vs[mid+1], U0V0[2*mid+1], U1V1[2*mid+1],
	UsVs[2*mid+1], al[2*N+1];
	WORD aux[N+1], aux2[N+2];

	Sum(U, &U[mid], Us, mid);	//where &U[mid] is upper half of the array
	Sum(V, &V[mid], Vs, mid); 	//where &V[mid] is upper half of the array
	if (mid <= 8)
	{
		Mul(U, V, W, mid);
		Mul(&U[mid], &V[mid], &W[N], mid);
		Mul(Us, Vs, UsVs, mid+1);	//original
		/*Mul(Us, Vs, UsVs, mid);
		AND(Us, -Vs[mid], mid);
		Sum(&UsVs[mid], Us, aux,  mid);
		AND(Vs, -Us[mid], mid);
		Sum(aux, Vs, &UsVs[mid],  mid);
		UsVs[N] += (aux[mid] + (Us[mid] & Vs[mid]) );
		*/
	}else{
		Kar(U, V, W, mid);
		Kar(&U[mid], &V[mid], &W[N], mid);
		//Kar(Us, Us, UsVs, mid);
		Mul(Us, Vs, UsVs, mid);
		AND(Us, -Vs[mid], mid);
		Sum(&UsVs[mid], Us, aux,  mid);
		AND(Vs, -Us[mid], mid);
		Sum(aux, Vs, &UsVs[mid],  mid);
		UsVs[N] += (aux[mid] + (Us[mid] & Vs[mid]) );
	}

	Sum(W, &W[N], aux, 2*mid);			//(U0V0 + U1V1) 
	Sub(UsVs, aux, aux2, 2*mid+1);		//"USVS - (U0V0 + U1V1)" = USVS - U1V1 - U0V0
	Sum(&W[mid], aux2, al, 2*mid);		
	for ( i = 0; i < N; i++)
	{
		W[i+mid] = al[i] ;
	}
	W[mid+N] += al[N];

}

void Mul(const WORD* U, const WORD* V, WORD* W, int N){
	int i,j;
	DWORD P;
	WORD Pl, Ph, carry, c;
	Zeroes(W, 2*N);					//put zeroes to W
	// loop that make the school book multiplication
	for (i = 0; i < N; i++)
	{
		c = 0;						//another carry for most significant parts
		for (j = 0; j < N; j++)
		{
			carry = 0;				//carry for the less significant parts
			P = (DWORD)U[i] * V[j];	//U*V in a registe of 128 bits
			Pl = (WORD)P;			//less significant 64 bits of the result
			#if WORD_BITS == 64					
    			Ph = (WORD)(P >> 64);			//most significant 64 bits of the result
			#elif WORD_BITS == 32			
    			Ph = (WORD)(P >> 32);			//most significant 32 bits of the result
			#else
    			#error "Must select 32 / 64."
			#endif

			W[i+j] += Pl;			//adding Pl + bits of the result word
			if (W[i+j] < Pl){		//checking carry after the additon
				carry =1;			//if yes then
			}
			W[i+j] += c;			//addig 1 to the result in the position i+j
			if (W[i+j] < c){		//checking once more if carry occurred
				carry +=1;			//if yes then 
			}
			c = Ph + carry;			//adding carry to the most significant part
		}
		W[i+N] = c;					//carry for the most significant parts
	}
}

void MulB(const WORD* U, const WORD* V, WORD* W, int N, int M){
	int i,j;
	DWORD P;
	WORD Pl, Ph, carry, c;
	Zeroes(W, N+M);
	// loop that made the school book multiplication
	for (i = 0; i < N; i++)
	{
		c = 0;						//another carry for most significant parts
		for (j = 0; j < M; j++)
		{
			carry = 0;				//carry for the less significant parts
			P = (DWORD)U[i] * V[j];	//U*V in a registe of 128 bits
			Pl = (WORD)P;			//less significant 64 bits of the result
			#if WORD_BITS == 64
    			Ph = (WORD)(P >> 64);			//most significant 64 bits of the result
			#elif WORD_BITS == 32
    			Ph = (WORD)(P >> 32);			//most significant 32 bits of the result
			#else
    			#error "Must select 32 / 64."
			#endif

			W[i+j] += Pl;			//adding Pl + bits of the result word
			if (W[i+j] < Pl){		//checking carry after the additon
				carry =1;			//if yes then
			}
			W[i+j] += c;			//addig 1 to the result in the position i+j
			if (W[i+j] < c){		//checking once more if carry occurred
				carry +=1;			//if yes then 
			}
			c = Ph + carry;			//adding carry to the most significant part
		}
		W[i+M] = c;					//carry for the most significant parts
	}
}

void Sum(const WORD* U, const WORD* V, WORD* W, int N){
    int i;
    WORD carry = 0;
    for (i = 0; i < N; i++)
    {
        WORD s1 = U[i] + V[i];
        WORD c1 = (s1 < U[i]);
        WORD s2 = s1 + carry;
        WORD c2 = (s2 < s1);
        W[i] = s2;
        carry = (WORD)(c1 | c2);
    }
    W[i] = carry;
}

void SubMod(const WORD* U, const WORD* V, WORD* W, const WORD* P, int N){
	int i;
	WORD C[N+1];
	uint64_t borrow = 0; //uint64_t borrow = 0; //original line of 64-bit implementation 
	for ( i = 0; i < N; i++)
	{
		W[i] = U[i] - V[i] - borrow;
		if (W[i] > U[i]){
			borrow = 1;
		}else{
			borrow = 0;
		}
	}
	if (borrow == 1)
	{	
		Sum(P, W, C, N);
		Cpy(W, C, N);
	}
}

void Sub(const WORD* Ut, const WORD* Vt, WORD* W, int N){

	int i;
	const WORD *U, *V;
	U = Ut;
	V = Vt;
	i = Cmp(Ut, Vt, N);				//compare Ut, Vt
	if (i == -1){					//if Vt > Ut then
		U = Vt;						// U -> Vt
		V = Ut;						// V -> Ut
	}
	WORD borrow = 0;
	for ( i = 0; i < N; i++)
	{
		W[i] = U[i] - V[i] - borrow;
		if (W[i] > U[i]){
			borrow = 1;
		}else{
			borrow = 0;
		}
	}
	W[i] = borrow;
}

int Cmp(const WORD* U, const WORD* V, int N){
	int i;
	for (i = N-1; i >= 0; i--)
	{
		if (U[i] != V[i] )
		{
			if(U[i] > V[i]) return 1;
			else return -1;
		}
	}
	return 0;
}

void Cpy( WORD* U, const WORD *V, int N){
	
	int i;
	for (int i = 0; i < N; i++)
	{
		U[i] = V[i];
	}
}

void Ran(WORD* U, WORD* V, int N){
    int i;
#if WORD_BITS == 64
    for (i = 0; i < N; i++)
    {
        U[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
        V[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
#elif WORD_BITS == 32
    for (i = 0; i < N; i++)
    {
        U[i] = (uint32_t)rand();
        V[i] = (uint32_t)rand();
    }
#else
    #error "Must select 32 / 64."
#endif
}

void XGCD(WORD* P, WORD* A, WORD* Inv, int N){
	int i;
	WORD u[N], v[N], x1[N], x2[N], aux[N+1];
	Cpy(u, A, N);
	Cpy(v, P, N);
	Zeroes(x1, N);
	x1[0] = 0x1;
	Zeroes(x2, N);
	while ((u[0] != 0x1) && (v[0] != 0x1)){
		while (!(u[0] & 0x1)){
			DivTwo(u, aux, N);
			Cpy(u, aux, N);
			if (!(x1[0] & 0x1))
			{
				DivTwo(x1, aux, N);
				Cpy(x1, aux,N);
			}else{
				Sum(x1, P, aux, N);
				DivTwo(aux, x1, N);
			}
		}
		while (!(v[0] & 0x1)){
			DivTwo(v, aux, N);
			Cpy(v, aux, N);
			if (!(x2[0] & 0x1))
			{
				DivTwo(x2, aux, N);
				Cpy(x2, aux, N);
			}else{
				Sum(x2, P, aux, N);
				DivTwo(aux, x2, N);
			}
		}
		if (Cmp(u, v, N) >= 0)
		{
			SubMod(u, v, aux, P, N);
			Cpy(u, aux, N);
			SubMod(x1, x2, aux, P, N);
			Cpy(x1, aux, N);

		}else{
			SubMod(v, u, aux, P, N);
			Cpy(v, aux, N);
			SubMod(x2, x1, aux, P, N);
			Cpy(x2, aux, N);
		}
	}
	if (u[0] == 0x1)
	{
		Cpy(Inv, x1, N);
	}else{
		Cpy(Inv, x2, N);
	}
}

void DivTwo(WORD *U, WORD *V, int N){
    int i;
    WORD lsb;
    for (i = 0; i < N - 1; i++)
    {
        lsb = U[i + 1] & (WORD)0x1;
        V[i] = (U[i] >> 1) | (lsb << (WORD_BITS - 1));
    }
    V[N - 1] = U[N - 1] >> 1;
}

void Barr(const WORD* A, const WORD* P, WORD* R, WORD* Red, int N){
	WORD q[2*N+2];
	WORD r2[2*N+1];
	WORD rp[N+2];
	WORD aux[N+1];

	Mul(&A[N-1], R, q, N+1);
	MulB(P, &q[N+1], r2, N, N+1);
	Sub(A, r2, rp, N+1);//
	while ( Cmp(rp, P, N) >= 0){
		Sub(rp, P, aux, N);
		Cpy(rp, aux, N);	
	}
	Cpy(Red, rp, N);
}
#include <stdio.h>

int g, ee, k;
int z = 1;

int main(){
	scanf("%d",&ee);
	for(g=ee;ee;ee-=!k)
		for(k=z++;z%k--;);
	printf("%i\n", z-g );
	return z-g;
}
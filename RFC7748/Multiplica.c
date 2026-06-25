#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int Multiply(int n, int m){
	int ans = 0, count = 0;
	while(m){
		if (m % 2 == 1){
			ans += n << count;
		}
		count ++;
		m /= 2;
	}
	return ans;
}

int main(int argc, char const *argv[])
{
	int c;
	printf("argc = %d \n", argc );

	for (int i = 0; i < argc; i++)
	{
		printf("argv[%d] = %s \n", i, argv[i] );
	}

	c = Multiply(atoi(argv[1]), atoi(argv[2]));

	//c = Multiply(a, b);
	printf("%d*%d = %d\n", atoi(argv[1]), atoi(argv[2]), c );

	return 0;
}
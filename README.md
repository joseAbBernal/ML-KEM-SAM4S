# En el directorio ml-kem-512/clean:

# 1. Compilar los archivos necesarios
gcc -O3 -Wall -Wextra -Wpedantic -std=c99 -c fips202.c randombytes.c

# 2. Compilar el main y linkar
gcc -O3 -Wall -Wextra -Wpedantic -std=c99 mlkem512.c fips202.o randombytes.o -L. -lml-kem-512_clean -o test_mlkem512

# 3. Ejecutar
./test_mlkem512


#O en un solo paso (sin library precompilada):
gcc -O3 -Wall -Wextra -Wpedantic -std=c99 mlkem512.c fips202.c randombytes.c -o test_mlkem512


Los archivos necesarios en el directorio:

mlkem512.c (tu main)
fips202.c / fips202.h (SHAKE)
randombytes.c / randombytes.h (entropía)
Y todos los .c / .h del proyecto ML-KEM-512

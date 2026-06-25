flags = -I./include -Wdiscarded-qualifiers -DWORD_BITS=64
VPATH = src include

all: test bench

functions.o: types.h functions.c
	gcc ${flags} -c src/functions.c -o functions.o

testSum.o: types.h testSum.c
	gcc ${flags} -c src/testSum.c -o testSum.o

Benching.o: Bench.h Bench.c
	gcc ${flags} -c src/Bench.c -o Benching.o

Benchmaking.o: Bench.h types.h BenchMaking.c
	gcc ${flags} -c src/BenchMaking.c -o Benchmaking.o

bench: functions.o Benching.o Benchmaking.o
	gcc ${flags} functions.o Benching.o Benchmaking.o -o bench

test: functions.o testSum.o	
	gcc ${flags} functions.o testSum.o -o test

clean:
	rm *.o test bench

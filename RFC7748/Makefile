CFLAGS = -I./include -Wdiscarded-qualifiers -DWORD_BITS=$(WORD_BITS)
VPATH = src include

.PHONY: all clean help 32 64 check-word-bits

.DEFAULT_GOAL := help

help:
	@echo "Uso:"
	@echo "  make 32"
	@echo "  make 64"
	@echo "  make all WORD_BITS=32"
	@echo "  make all WORD_BITS=64"

32:
	@$(MAKE) all WORD_BITS=32

64:
	@$(MAKE) all WORD_BITS=64

check-word-bits:
	@if [ "$(WORD_BITS)" != "32" ] && [ "$(WORD_BITS)" != "64" ]; then \
		echo "Error: WORD_BITS debe ser 32 o 64."; \
		echo "Uso correcto: make 32 | make 64 | make all WORD_BITS=32|64"; \
		exit 1; \
	fi

all: check-word-bits test bench

functions.o: types.h functions.c
	gcc $(CFLAGS) -c src/functions.c -o functions.o

testSum.o: types.h testSum.c
	gcc $(CFLAGS) -c src/testSum.c -o testSum.o

Benching.o: Bench.h Bench.c
	gcc $(CFLAGS) -c src/Bench.c -o Benching.o

Benchmaking.o: Bench.h types.h BenchMaking.c
	gcc $(CFLAGS) -c src/BenchMaking.c -o Benchmaking.o

bench: check-word-bits functions.o Benching.o Benchmaking.o
	gcc $(CFLAGS) functions.o Benching.o Benchmaking.o -o bench

test: check-word-bits functions.o testSum.o	
	gcc $(CFLAGS) functions.o testSum.o -o test

clean:
	rm -f *.o test bench

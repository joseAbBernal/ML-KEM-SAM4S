# This Makefile can be used with GNU Make or BSD Make

LIB=libml-kem-512_clean.a
HEADERS=api.h cbd.h indcpa.h kem.h ntt.h params.h poly.h polyvec.h reduce.h symmetric.h verify.h 
OBJECTS=cbd.o indcpa.o kem.o ntt.o poly.o polyvec.o reduce.o symmetric-shake.o verify.o fips202.o randombytes.o

CFLAGS=-O3 -Wall -Wextra -Wpedantic -Werror -Wmissing-prototypes -Wredundant-decls -std=c99 -I../../../common $(EXTRAFLAGS)

all: $(LIB) mlkem512

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

mlkem512: mlkem512.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ mlkem512.c $(OBJECTS)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIB)
	$(RM) mlkem512

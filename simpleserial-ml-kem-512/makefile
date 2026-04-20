# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
#
# Makefile for ChipWhisperer SimpleSerial-ml-kem-512 Program
#
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF.
#
# make extcoff = Convert ELF to AVR Extended COFF.
#
# make program = Download the hex file to the device, using avrdude.
#                Please customize the avrdude settings below first!
#
# make debug = Start either simulavr or avarice as specified for debugging,
#              with avr-gdb or avr-insight as the front end for debugging.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

# Target file name (without extension).
# This is the base name of the compiled .hex file.
#TARGET = simpleserial-aes
TARGET = simpleserial-ml-kem-512

# List C source files here.
# Header files (.h) are automatically pulled in.
#CPPSRC += simpleserial-aes.cpp
#CPPSRC += simpleserial-ml-kem-512.cpp
#SRC += simpleserial-aes.c
SRC += simpleserial-ml-kem-512.c
SRC += kem.c indcpa.c cbd.c ntt.c poly.c polyvec.c reduce.c symmetric-shake.c verify.c fips202.c randombytes_sam4s.c

# -----------------------------------------------------------------------------
EXTRA_OPTS = NO_EXTRA_OPTS
CFLAGS += -D$(EXTRA_OPTS)


ifeq ($(CRYPTO_TARGET),)
  ${info No CRYPTO_TARGET passed - defaulting to TINYAES128C}
  CRYPTO_TARGET = TINYAES128C
endif

${info Building for platform ${PLATFORM} with CRYPTO_TARGET=$(CRYPTO_TARGET)}

ifeq ($(SS_VER),)
	SS_VER = SS_VER_2_1
endif

#Add simpleserial project to build
include ../simpleserial/Makefile.simpleserial

FIRMWAREPATH = ../.
include $(FIRMWAREPATH)/Makefile.inc

# ============================================================================
# ML-KEM-512 Library Build Configuration
# ============================================================================

LIB=libml-kem-512_clean.a
HEADERS=api.h cbd.h indcpa.h kem.h ntt.h params.h poly.h polyvec.h reduce.h symmetric.h verify.h 
OBJECTS=cbd.o indcpa.o kem.o ntt.o poly.o polyvec.o reduce.o symmetric-shake.o verify.o 

ML_KEM_CFLAGS=-O3 -Wall -Wextra -Wpedantic -Werror -Wmissing-prototypes -Wredundant-decls -std=c99 -I../../../common $(EXTRAFLAGS)

# ML-KEM Library targets
%.o: %.c $(HEADERS)
	$(CC) $(ML_KEM_CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

# Update clean target to include ML-KEM objects and library
clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIB)


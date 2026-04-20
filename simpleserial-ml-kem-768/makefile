# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
#
# Makefile for ChipWhisperer SimpleSerial-ML-KEM-768 Program
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
TARGET = simpleserial-ml-kem-768

# List C source files here.
# Header files (.h) are automatically pulled in.
SRC += simpleserial-ml-kem-768.c
SRC += kem.c
SRC += indcpa.c
SRC += poly.c
SRC += polyvec.c
SRC += ntt.c
SRC += fips202.c
SRC += keccakf1600.c
SRC += symmetric-fips202.c
SRC += verify.c
SRC += cbd.c
SRC += randombytes.c
SRC += reduce.c

# Assembly files (use ASRC for .S files)
# NOTE: For SAM4S without FPU, using only non-FPU assembly
ASRC += cmov_int16.S
ASRC += fastaddsub.S

# -----------------------------------------------------------------------------
EXTRA_OPTS = NO_EXTRA_OPTS
CFLAGS += -D$(EXTRA_OPTS)


ifeq ($(CRYPTO_TARGET),)
  ${info No CRYPTO_TARGET passed - defaulting to NONE}
  CRYPTO_TARGET = NONE
endif

${info Building for platform ${PLATFORM} with CRYPTO_TARGET=$(CRYPTO_TARGET)}

#Add simpleserial project to build
include ../simpleserial/Makefile.simpleserial

FIRMWAREPATH = ../.
include $(FIRMWAREPATH)/Makefile.inc

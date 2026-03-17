# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
#
# Makefile for ChipWhisperer SimpleSerial-ML-KEM-512 Program
#
#----------------------------------------------------------------------------

# Target file name (without extension).
TARGET = simpleserial-mk-kem-512

# List C source files here.
#CPPSRC += simpleserial-ml-kem-512.cpp
SRC += simpleserial-ml-kem-512.c

# ML-KEM-512 source files (from current directory)
SRC += kem.c
SRC += poly.c
SRC += polyvec.c
SRC += ntt.c
SRC += indcpa.c
SRC += fips202.c
SRC += keccakf1600.c
SRC += verify.c
SRC += cbd.c
SRC += matacc.c
SRC += randombytes.c
SRC += symmetric-fips202.c
SRC += ml-kem-stubs.c

# Assembly files - REMOVED because they require FPU on SAM4S
# ASRC += reduce.S
# ASRC += fastntt.S
# ASRC += fastinvntt.S
# ASRC += fastbasemul.S
# ASRC += fastaddsub.S
# ASRC += poly_asm.S
# ASRC += cmov_int16.S
# ASRC += matacc_asm.S

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

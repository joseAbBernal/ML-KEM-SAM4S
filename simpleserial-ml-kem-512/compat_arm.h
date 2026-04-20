#ifndef  PQCLEAN_COMMON_COMPAT_ARM_H
#define PQCLEAN_COMMON_COMPAT_ARM_H

/* This file serves to solve compatibility issues between different
 * versions of compilers for ARM targets.
 *
 * This file is allowed to use #ifdefs and toggle things by compiler versions.
 */

/* For ARM targets, we skip the complex GCC version checks and features.h inclusion
   that cause conflicts with the ARM compiler and _fake_typedefs.h */

/************************
 * Portable VLA support *
 ************************/

/* To support MSVC use alloca() instead of VLAs. */
#ifdef _MSC_VER
/* MSVC defines _alloca in malloc.h */
# include <malloc.h>
/* Note: _malloca(), which is recommended over deprecated _alloca,
   requires that you call _freea(). So we stick with _alloca */
# define PQCLEAN_VLA(__t,__x,__s) __t *__x = (__t*)_alloca((__s)*sizeof(__t))
#else
# define PQCLEAN_VLA(__t,__x,__s) __t __x[__s]
#endif

/*********************************
 * Prevent branching on variable *
 *********************************/

#if defined(__GNUC__) || defined(__clang__)
  // Prevent the compiler from
  //    1) inferring that b is 0/1-valued, and
  //    2) handling the two cases with a branch.
  // This is not necessary when verify.c and kem.c are separate translation
  // units, but we expect that downstream consumers will copy this code and/or
  // change how it is built.
# define PQCLEAN_PREVENT_BRANCH_HACK(b)  __asm__("" : "+r"(b) : /* no inputs */);
#else
# define PQCLEAN_PREVENT_BRANCH_HACK(b)
#endif

#endif // PQCLEAN_COMMON_COMPAT_ARM_H
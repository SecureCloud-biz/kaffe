/* Copyright (C) 2005 Thiemo Seufer
   Atomic functions for MIPS

   This file is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version. */

/* Modified to get it work on all kaffe's platforms. */
#include "config-int.h"
#include <sgidefs.h>

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/*
 * MIPS does not have byte and halfword forms of load linked and store
 * conditional. So for MIPS we stub out the 8- and 16-bit forms.
 */
#define __arch_compare_and_exchange_bool_8_int(mem, newval, oldval, mb1, mb2)	\
  ({ __builtin_trap (); 0; })

#define __arch_compare_and_exchange_bool_16_int(mem, newval, oldval, mb1, mb2)	\
  ({ __builtin_trap (); 0; })

#define __arch_compare_and_exchange_val_8_int(mem, newval, oldval, mb1, mb2)	\
  ({ __builtin_trap (); 0; })

#define __arch_compare_and_exchange_val_16_int(mem, newval, oldval, mb1, mb2)	\
  ({ __builtin_trap (); 0; })

#ifdef UP
# define __MB		/* nothing */
#else
# define __MB		"	sync\n"
# define atomic_full_barrier()	__asm ("sync" : : : "memory");
# define atomic_read_barrier()	__asm ("sync" : : : "memory");
# define atomic_write_barrier()	__asm ("sync" : : : "memory");
#endif

/* Compare and exchange.  For all of the "xxx" routines, we expect a
   "__prev" and a "__cmp" variable to be provided by the enclosing scope,
   in which values are returned.  */

#define __arch_compare_and_exchange_xxx_32_int(mem, new, old, mb1, mb2) \
({									\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__prev],%[__mem]\n"				\
	"	bne	%[__prev],%[__old],2f\n"			\
	"	 li	%[__cmp],0\n"					\
	"	move	%[__cmp],%[__new]\n"				\
	"	sc	%[__cmp],%[__mem]\n"				\
	"	beqz	%[__cmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"2:\n"								\
	"	.set pop"						\
	: [__prev] "=&r" (__prev),					\
	  [__cmp] "=&r" (__cmp)						\
	: [__mem] "R" (*(volatile atomic32_t *)(mem)),			\
	  [__old] "r" ((atomic32_t)(old)),				\
	  [__new] "r" ((atomic32_t)(new))				\
	: "memory");							\
})

#define __arch_compare_and_exchange_xxx_64_int(mem, new, old, mb1, mb2)	\
({									\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__prev],%[__mem]\n"				\
	"	bne	%[__prev],%[__old],2f\n"			\
	"	 li	%[__cmp],0\n"					\
	"	move	%[__cmp],%[__new]\n"				\
	"	scd	%[__cmp],%[__mem]\n"				\
	"	beqz	%[__cmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"2:\n"								\
	"	.set pop"						\
	: [__prev] "=&r" (__prev),					\
	  [__cmp] "=&r" (__cmp)						\
	: [__mem] "R" (*(volatile atomic32_t *)(mem)),			\
	  [__old] "r" ((atomic64_t)(old)),				\
	  [__new] "r" ((atomic64_t)(new))				\
	: "memory");							\
})

/* For all "bool" routines, we return FALSE if exchange successful.  */

#define __arch_compare_and_exchange_bool_32_int(mem, new, old, mb1, mb2) \
({ unsigned long __prev; int __cmp;					\
   __arch_compare_and_exchange_xxx_32_int(mem, new, old, mb1, mb2);	\
   !__cmp; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_compare_and_exchange_bool_64_int(mem, new, old, mb1, mb2) \
  ({ __builtin_trap (); 0; })
#else
# define __arch_compare_and_exchange_bool_64_int(mem, new, old, mb1, mb2) \
({ unsigned long __prev; int __cmp;					\
   __arch_compare_and_exchange_xxx_64_int(mem, new, old, mb1, mb2);	\
   !__cmp; })
#endif

/* For all "val" routines, return the old value whether exchange
   successful or not.  */

#define __arch_compare_and_exchange_val_32_int(mem, new, old, mb1, mb2) \
({ unsigned long __prev; int __cmp;					\
   __arch_compare_and_exchange_xxx_32_int(mem, new, old, mb1, mb2);	\
   (typeof (*mem))__prev; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_compare_and_exchange_val_64_int(mem, new, old, mb1, mb2) \
  ({ __builtin_trap (); 0; })
#else
# define __arch_compare_and_exchange_val_64_int(mem, new, old, mb1, mb2) \
({ unsigned long __prev; int __cmp;					\
   __arch_compare_and_exchange_xxx_64_int(mem, new, old, mb1, mb2);	\
   (typeof (*mem))__prev; })
#endif

/* Compare and exchange with "acquire" semantics, ie barrier after.  */

#define atomic_compare_and_exchange_bool_acq(mem, new, old)		\
  __atomic_bool_bysize (__arch_compare_and_exchange_bool, int,		\
		        mem, new, old, "", __MB)

#define atomic_compare_and_exchange_val_acq(mem, new, old)		\
  __atomic_val_bysize (__arch_compare_and_exchange_val, int,		\
		       mem, new, old, "", __MB)

/* Compare and exchange with "release" semantics, ie barrier before.  */

#define atomic_compare_and_exchange_bool_rel(mem, new, old)		\
  __atomic_bool_bysize (__arch_compare_and_exchange_bool, int,		\
		        mem, new, old, __MB, "")

#define atomic_compare_and_exchange_val_rel(mem, new, old)		\
  __atomic_val_bysize (__arch_compare_and_exchange_val, int,		\
		       mem, new, old, __MB, "")


/* Atomically store value and return the previous value.  */

#define __arch_exchange_8_int(mem, value, mb1, mb2)			\
  ({ __builtin_trap (); 0; })

#define __arch_exchange_16_int(mem, value, mb1, mb2)			\
  ({ __builtin_trap (); 0; })

#define __arch_exchange_32_int(mem, value, mb1, mb2)			\
({									\
  unsigned int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__ret],%[__mem]\n"				\
	"	move	%[__tmp],%[__val]\n"				\
	"	sc	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__val] "r" ((unsigned int)(value))				\
	: "memory");							\
  __ret; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_exchange_64_int(mem, value, mb1, mb2)			\
  ({ __builtin_trap (); 0; })
#else
# define __arch_exchange_64_int(mem, value, mb1, mb2)			\
({									\
  unsigned long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__ret],%[__mem]\n"				\
	"	move	%[__tmp],%[__val]\n"				\
	"	scd	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__val] "r" (value)						\
	: "memory");							\
  __ret; })
#endif

#define atomic_exchange_acq(mem, value)					\
  __atomic_val_bysize (__arch_exchange, int, mem, value, "", __MB)

#define atomic_exchange_rel(mem, value)					\
  __atomic_val_bysize (__arch_exchange, int, mem, value, __MB, "")


/* Atomically add value and return the previous (unincremented) value.  */

#define __arch_exchange_and_add_8_int(mem, value, mb1, mb2)		\
  ({ __builtin_trap (); 0; })

#define __arch_exchange_and_add_16_int(mem, value, mb1, mb2)		\
  ({ __builtin_trap (); 0; })

#define __arch_exchange_and_add_32_int(mem, value, mb1, mb2)		\
({									\
  unsigned int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__ret],%[__mem]\n"				\
	"	addu	%[__tmp],%[__val],%[__ret]\n"			\
	"	sc	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__val] "r" ((unsigned int)(value))				\
	: "memory");							\
  __ret; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_exchange_and_add_64_int(mem, value, mb1, mb2)		\
  ({ __builtin_trap (); 0; })
#else
# define __arch_exchange_and_add_64_int(mem, value, mb1, mb2)		\
({									\
  unsigned long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__ret],%[__mem]\n"				\
	"	daddu	%[__tmp],%[__val],%[__ret]\n"			\
	"	scd	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__val] "r" ((unsigned long)(value))				\
	: "memory");							\
  __ret; })
#endif

/* ??? Barrier semantics for atomic_exchange_and_add appear to be 
   undefined.  Use full barrier for now, as that's safe.  */
#define atomic_exchange_and_add(mem, value)				\
  __atomic_val_bysize (__arch_exchange_and_add, int, mem, value, __MB, __MB)


/* Atomically increment value (and return the incremented value).  */

#define __arch_increment_8_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })

#define __arch_increment_16_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })

#define __arch_increment_32_int(mem, mb1, mb2)				\
({									\
  unsigned int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__ret],%[__mem]\n"				\
	"	addiu	%[__tmp],%[__ret],1\n"				\
	"	sc	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 addiu	%[__ret],%[__ret],1\n"				\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_increment_64_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })
#else
# define __arch_increment_64_int(mem, mb1, mb2)				\
({									\
  unsigned long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__ret],%[__mem]\n"				\
	"	daddiu	%[__tmp],%[__ret],1\n"				\
	"	scd	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 daddiu	%[__ret],%[__ret],1\n"				\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })
#endif

/* ??? Barrier semantics for atomic_increment_val appear to be 
   undefined.  Use full barrier for now, as that's safe.  */
#define atomic_increment_val(mem)					\
  __atomic_val_bysize (__arch_increment, int, mem, __MB, __MB)

#define atomic_increment(mem)						\
  ({ __atomic_val_bysize (__arch_increment, int, mem, "", ""); (void) 0; })


/* Atomically decrement value (and return the decremented value).  */

#define __arch_decrement_8_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })

#define __arch_decrement_16_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })

#define __arch_decrement_32_int(mem, mb1, mb2)				\
({									\
  unsigned int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__ret],%[__mem]\n"				\
	"	addiu	%[__tmp],%[__ret],-1\n"				\
	"	sc	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 addiu	%[__ret],%[__ret],-1\n"				\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_decrement_64_int(mem, mb1, mb2)				\
  ({ __builtin_trap (); 0; })
#else
# define __arch_decrement_64_int(mem, mb1, mb2)				\
({									\
  unsigned long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__ret],%[__mem]\n"				\
	"	daddiu	%[__tmp],%[__ret],-1\n"				\
	"	scd	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 daddiu	%[__ret],%[__ret],-1\n"				\
		mb2							\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })
#endif

/* ??? Barrier semantics for atomic_decrement_val appear to be 
   undefined.  Use full barrier for now, as that's safe.  */
#define atomic_decrement_val(mem)					\
  __atomic_val_bysize (__arch_decrement, int, mem, __MB, __MB)

#define atomic_decrement(mem)						\
  ({ __atomic_val_bysize (__arch_decrement, int, mem, "", ""); (void) 0; })


/* Atomically decrement value and return the previous (undecremented) value.  */

#define __arch_decrement_if_positive_8_int(mem, mb1, mb2)		\
  ({ __builtin_trap (); 0; })

#define __arch_decrement_if_positive_16_int(mem, mb1, mb2)		\
  ({ __builtin_trap (); 0; })

#define __arch_decrement_if_positive_32_int(mem, mb1, mb2)		\
({									\
  signed int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__ret],%[__mem]\n"				\
	"	addiu	%[__tmp],%[__ret],-1\n"				\
	"	bltz	%[__tmp],2f\n"					\
	"	 nop\n"							\
	"	sc	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"2:\n"								\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_decrement_if_positive_64_int(mem, mb1, mb2)		\
  ({ __builtin_trap (); 0; })
#else
# define __arch_decrement_if_positive_64_int(mem, mb1, mb2)		\
({									\
  signed long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__ret],%[__mem]\n"				\
	"	daddiu	%[__tmp],%[__ret],-1\n"				\
	"	bltz	%[__tmp],2f\n"					\
	"	 nop\n"							\
	"	scd	%[__tmp],%[__mem]\n"				\
	"	beqz	%[__tmp],1b\n"					\
	"	 nop\n"							\
		mb2							\
	"2:\n"								\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem)						\
	: "memory");							\
  __ret; })
#endif

#define atomic_decrement_if_positive(mem)				\
  __atomic_val_bysize (__arch_decrement_if_positive, int, mem, "", __MB)


/* Atomically set a bit and return its old value.  */

#define __arch_bit_test_set_8_int(mem, bit, mb1, mb2)			\
  ({ __builtin_trap (); 0; })

#define __arch_bit_test_set_16_int(mem, bit, mb1, mb2)			\
  ({ __builtin_trap (); 0; })

#define __arch_bit_test_set_32_int(mem, bit, mb1, mb2)			\
({									\
  unsigned int __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips2\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	ll	%[__tmp],%[__mem]\n"				\
	"	or	%[__ret],%[__tmp],%[__bit]\n"			\
	"	sc	%[__ret],%[__mem]\n"				\
	"	beqz	%[__ret],1b\n"					\
	"	 and	%[__ret],%[__tmp],%[__bit]\n"			\
		mb2							\
	"2:\n"								\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__bit] "r" (1UL << bit)					\
	: "memory");							\
  __ret != 0; })

#if _MIPS_SIM == _MIPS_SIM_ABI32
# define __arch_bit_test_set_64_int(mem, bit, mb1, mb2)			\
  ({ __builtin_trap (); 0; })
#else
# define __arch_bit_test_set_64_int(mem, bit, mb1, mb2)			\
({									\
  unsigned long __ret, __tmp;						\
  __asm__ __volatile__ (						\
	"	.set push\n"						\
	"	.set mips3\n"						\
	"	.set noreorder\n"					\
		mb1							\
	"1:	lld	%[__tmp],%[__mem]\n"				\
	"	or	%[__ret],%[__tmp],%[__bit]\n"			\
	"	scd	%[__ret],%[__mem]\n"				\
	"	beqz	%[__ret],1b\n"					\
	"	 and	%[__ret],%[__tmp],%[__bit]\n"			\
		mb2							\
	"2:\n"								\
	"	.set pop\n"						\
	: [__ret] "=&r" (__ret),					\
	  [__tmp] "=&r" (__tmp)						\
	: [__mem] "R" (*mem),						\
	  [__bit] "r" (1 << bit)					\
	: "memory");							\
  __ret; })
#endif

#define atomic_bit_test_set(mem)					\
  __atomic_val_bysize (__arch_bit_test_set, int, mem, "", __MB)

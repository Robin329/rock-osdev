/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

/* Architecture-specific global data */
struct arch_global_data {

	/* "static data" needed by most of timer.c on ARM platforms */
	unsigned long timer_rate_hz;
	unsigned int tbu;
	unsigned int tbl;
	unsigned long lastinc;
	unsigned long long timer_reset_value;
    unsigned long tlb_addr;
	unsigned long tlb_size;
	unsigned long tlb_fillptr;
	unsigned long tlb_emerg;
};

#include <asm-generic/global_data.h>

#ifdef __clang__

#define DECLARE_GLOBAL_DATA_PTR
#define gd	get_gd()

static inline gd_t *get_gd(void)
{
	gd_t *gd_ptr;

#ifdef CONFIG_ARM64
	/*
	 * Make will already error that reserving x18 is not supported at the
	 * time of writing, clang: error: unknown argument: '-ffixed-x18'
	 */
	__asm__ volatile("mov %0, x18\n" : "=r" (gd_ptr));
#else
	__asm__ volatile("mov %0, r9\n" : "=r" (gd_ptr));
#endif

	return gd_ptr;
}

#else

#ifdef CONFIG_ARM64
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("x18")
#else
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("r9")
#endif
#endif

#endif /* __ASM_GBL_DATA_H */

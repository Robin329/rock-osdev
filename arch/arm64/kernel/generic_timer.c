// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Generic timer implementation of get_tbclk()
 */
unsigned long get_tbclk(void)
{
	unsigned long cntfrq;
	asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq));
	return cntfrq;
}

void set_tbclk(unsigned long cntfrq)
{
    asm volatile("msr cntfrq_el0, %x0": : "rZ" (cntfrq));
	isb();
}

/*
 * timer_read_counter() using the Arm Generic Timer (aka arch timer).
 */
unsigned long timer_read_counter(void)
{
	unsigned long cntpct;

	isb();
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));

	return cntpct;
}

uint64_t get_ticks(void)
{
	unsigned long ticks = timer_read_counter();

	gd->arch.tbl = ticks;

	return ticks;
}

unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;
	if (usec < 1000)
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	else
		ticks = ((usec / 10) * (get_tbclk() / 100000));

	return ticks;
}

ulong timer_get_boot_us(void)
{
	u64 val = get_ticks() * 1000000;

	return val / get_tbclk();
}

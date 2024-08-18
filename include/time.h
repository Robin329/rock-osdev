/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _TIME_H
#define _TIME_H

#include <linux/typecheck.h>
#include <linux/types.h>

ulong get_tbclk(void);
void set_tbclk(unsigned long);

unsigned long get_timer(unsigned long base);

/*
 * Return the current value of a monotonically increasing microsecond timer.
 * Granularity may be larger than 1us if hardware does not support this.
 */
uint64_t get_timer_us(uint64_t base);

/*
 * timer_test_add_offset()
 *
 * Allow tests to add to the time reported through lib/time.c functions
 * offset: number of milliseconds to advance the system time
 */
void timer_test_add_offset(unsigned long offset);

/**
 * usec_to_tick() - convert microseconds to clock ticks
 *
 * @usec:	duration in microseconds
 * Return:	duration in clock ticks
 */
uint64_t usec_to_tick(unsigned long usec);

/*
 *	These inlines deal with timer wrapping correctly. You are
 *	strongly encouraged to use them
 *	1. Because people otherwise forget
 *	2. Because if the timer wrap changes in future you won't have to
 *	   alter your driver code.
 *
 * time_after(a,b) returns true if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */
#define time_after(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((b) - (a)) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((a) - (b)) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

/*
 * Calculate whether a is in the range of [b, c].
 */
#define time_in_range(a,b,c) \
	(time_after_eq(a,b) && \
	 time_before_eq(a,c))

/*
 * Calculate whether a is in the range of [b, c).
 */
#define time_in_range_open(a,b,c) \
	(time_after_eq(a,b) && \
	 time_before(a,c))

/**
 * usec2ticks() - Convert microseconds to internal ticks
 *
 * @usec: Value of microseconds to convert
 * @return Corresponding internal ticks value, calculated using get_tbclk()
 */
ulong usec2ticks(unsigned long usec);

/**
 * ticks2usec() - Convert internal ticks to microseconds
 *
 * @ticks: Value of ticks to convert
 * @return Corresponding microseconds value, calculated using get_tbclk()
 */
ulong ticks2usec(unsigned long ticks);

/**
 * wait_ticks() - waits a given number of ticks
 *
 * This is an internal function typically used to implement udelay() and
 * similar. Normally you should use udelay() or mdelay() instead.
 *
 * @ticks: Number of ticks to wait
 */
void wait_ticks(unsigned long ticks);

/**
 * timer_get_us() - Get monotonic microsecond timer
 *
 * @return value of monotonic microsecond timer
 */
unsigned long timer_get_us(void);

/**
 * get_ticks() - Get the current tick value
 *
 * This is an internal value used by the timer on the system. Ticks increase
 * monotonically at the rate given by get_tbclk().
 *
 * @return current tick value
 */
uint64_t get_ticks(void);

typedef long register_t;
typedef unsigned long u_register_t;

/**********************************************************************
 * Macros which create inline functions to read or write CPU system
 * registers
 *********************************************************************/

#define _DEFINE_SYSREG_READ_FUNC(_name, _reg_name)		\
static inline u_register_t read_ ## _name(void)			\
{								\
	u_register_t v;						\
	__asm__ volatile ("mrs %0, " #_reg_name : "=r" (v));	\
	return v;						\
}

#define _DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)			\
static inline void write_ ## _name(u_register_t v)			\
{									\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "r" (v));	\
}

/* Define read function for system register */
#define DEFINE_SYSREG_READ_FUNC(_name) 			\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)

/* Define read & write function for system register */
#define DEFINE_SYSREG_RW_FUNCS(_name)					\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)				\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _name)

DEFINE_SYSREG_RW_FUNCS(cntfrq_el0)
DEFINE_SYSREG_RW_FUNCS(cnthp_ctl_el2)
DEFINE_SYSREG_RW_FUNCS(cnthp_tval_el2)
DEFINE_SYSREG_RW_FUNCS(cnthp_cval_el2)
DEFINE_SYSREG_RW_FUNCS(cntps_ctl_el1)
DEFINE_SYSREG_RW_FUNCS(cntps_tval_el1)
DEFINE_SYSREG_RW_FUNCS(cntps_cval_el1)
DEFINE_SYSREG_RW_FUNCS(cntp_ctl_el0)
DEFINE_SYSREG_RW_FUNCS(cntp_tval_el0)
DEFINE_SYSREG_RW_FUNCS(cntp_cval_el0)
DEFINE_SYSREG_READ_FUNC(cntpct_el0)
DEFINE_SYSREG_RW_FUNCS(cnthctl_el2)
DEFINE_SYSREG_RW_FUNCS(cntv_ctl_el0)
DEFINE_SYSREG_RW_FUNCS(cntv_tval_el0)
DEFINE_SYSREG_RW_FUNCS(cntv_cval_el0)
DEFINE_SYSREG_RW_FUNCS(cnthv_ctl_el0)
DEFINE_SYSREG_RW_FUNCS(cnthv_cval_el0)
#define CNTHV_CTL_EL2	sys_reg(3, 4, 14, 3, 1)
#define CNTHV_CVAL_EL2	sys_reg(3, 4, 14, 3, 2)



/* Physical timer control register bit fields shifts and masks */
#define CNTP_CTL_ENABLE_SHIFT	(1U << 0)
#define CNTP_CTL_IMASK_SHIFT	(1U << 1)
#define CNTP_CTL_ISTATUS_SHIFT	(1U << 2)

#endif /* _TIME_H */

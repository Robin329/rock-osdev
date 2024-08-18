#ifndef __COMMON_H_
#define __COMMON_H_

#ifndef __ASSEMBLY__		/* put C only stuff in this section */
#include <errno.h>
#include <time.h>
#include <asm-offsets.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/kernel.h>
#include <asm/global_data.h>	/* global data used for startup functions */
#include <irq_func.h>
#include <vsprintf.h>
#include <rand.h>
#endif	/* __ASSEMBLY__ */

#ifdef __LP64__
#define CONFIG_SYS_SUPPORT_64BIT_DATA
#endif

#endif

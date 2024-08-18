/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __ASM_GENERIC_GBL_DATA_H
#define __ASM_GENERIC_GBL_DATA_H
/*
 * The following data structure is placed in some memory which is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set GENERATED_GBL_DATA_SIZE > sizeof(gd_t)
 *
 * Each architecture has its own private fields. For now all are private
 */

#ifndef __ASSEMBLY__
#include <configs.h>
#include <linux/list.h>

struct bd_info {
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size	 of FLASH memory */
	unsigned long	bi_sramstart;	/* start of SRAM memory */
	unsigned long	bi_sramsize;	/* size	 of SRAM memory */
#ifdef CONFIG_NR_DRAM_BANKS
	struct {			/* RAM configuration */
		phys_addr_t start;
		phys_size_t size;
	} bi_dram[CONFIG_NR_DRAM_BANKS];
#endif /* CONFIG_NR_DRAM_BANKS */
};

typedef struct global_data {
	unsigned long flags;
	unsigned int baudrate;
	unsigned long cpu_clk;		/* CPU clock in Hz!		*/
	unsigned long mem_clk;

	unsigned long ram_base;		/* Base address of RAM used by U-Boot */
	unsigned long ram_top;		/* Top address of RAM used by U-Boot */
	unsigned long relocaddr;	/* Start address of U-Boot in RAM */
	phys_size_t ram_size;		/* RAM size */
	unsigned long mon_len;		/* monitor len */
	unsigned long irq_sp;		/* irq stack pointer */
	unsigned long start_addr_sp;	/* start_addr_stackpointer */
	unsigned long reloc_off;
	unsigned long malloc_base;	/* base address of early malloc() */
	unsigned long malloc_limit;	/* limit address */
	struct global_data *new_gd;	/* relocated global data */

    struct arch_global_data arch;	/* architecture-specific data */
	struct bd_info bd;
} gd_t;
#endif


/*
 * Global Data Flags
 */
#define GD_FLG_RELOC		0x00001	/* Code was relocated to RAM	   */
#define GD_FLG_DEVINIT		0x00002	/* Devices have been initialized   */
#define GD_FLG_SILENT		0x00004	/* Silent mode			   */
#define GD_FLG_POSTFAIL		0x00008	/* Critical POST test failed	   */
#define GD_FLG_POSTSTOP		0x00010	/* POST seqeunce aborted	   */
#define GD_FLG_LOGINIT		0x00020	/* Log Buffer has been initialized */
#define GD_FLG_DISABLE_CONSOLE	0x00040	/* Disable console (in & out)	   */
#define GD_FLG_ENV_READY	0x00080	/* Env. imported into hash table   */
#define GD_FLG_SERIAL_READY	0x00100	/* Pre-reloc serial console ready  */
#define GD_FLG_FULL_MALLOC_INIT	0x00200	/* Full malloc() is ready	   */
#define GD_FLG_SPL_INIT		0x00400	/* spl_init() has been called	   */
#define GD_FLG_SKIP_RELOC	0x00800	/* Don't relocate		   */
#define GD_FLG_RECORD		0x01000	/* Record console		   */
#define GD_FLG_ENV_DEFAULT	0x02000 /* Default variable flag	   */
#define GD_FLG_SPL_EARLY_INIT	0x04000 /* Early SPL init is done	   */
#define GD_FLG_LOG_READY	0x08000 /* Log system is ready for use	   */
#define GD_FLG_WDT_READY	0x10000 /* Watchdog is ready for use	   */

#endif /* __ASM_GENERIC_GBL_DATA_H */

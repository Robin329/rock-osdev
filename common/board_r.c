// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <irq_func.h>
#include <command.h>
#include <initcall.h>
#include <irq_func.h>
#include <malloc.h>
#include <mapmem.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <initcall.h>
#include <asm/sections.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

static int initr_reloc(void)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	return 0;
}

#ifdef CONFIG_ARM
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
static int initr_caches(void)
{
	/* Enable caches */
	enable_caches();
	return 0;
}
#endif

__weak int board_early_init_r(void)
{
	return 0;
}

static int initr_enable_interrupts(void)
{
	enable_interrupts();
	return 0;
}

static int run_main_loop(void)
{
	/* main_loop() can return to retry autoboot, if so just run it again */
	puts("\n");
	for (;;)
		main_loop();
	return 0;
}

/*
 * We hope to remove most of the driver-related init and do it if/when
 * the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */
static init_fnc_t init_sequence_r[] = {
	initr_reloc,
	initr_caches,
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
	memory_init,
	interrupt_init,
	board_early_init_r,
	initr_enable_interrupts,
	run_main_loop,
};

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	gd->flags &= ~GD_FLG_LOG_READY;

	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}

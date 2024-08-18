/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <configs.h>

#if (CONFIG_RELOC_SYS_TEXT_BASE < CONFIG_SYSMEM_DDR0_START)
#error "Armv8 multientry cpus need run on sdram"
#endif

#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/mmu.h>

ENTRY(cpus_secondary_jump)
.globl cpus_reserve_begin
cpus_reserve_begin:
    bl	c_runtime_cpu_setup		/* still call old routine */

    mrs	x1, mpidr_el1
    and x1, x1, #16777215
    mov x1, x1, lsl 8           /* 64KB sp per cpu */

	ldr	x0, =CONFIG_SYS_INIT_SP_ADDR //[x18, #GD_START_ADDR_SP]	/* x0 <- gd->start_addr_sp */
    sub x0, x0, x1
	bic	sp, x0, #0xf	/* 16-byte alignment for ABI compliance */

    mov x0, x1
    bl jump_cpus

loop:
    b loop

.globl cpus_reserve_end
cpus_reserve_end:
ENDPROC(cpus_secondary_jump)

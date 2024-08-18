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
#include <configs.h>
#include <hang.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <initcall.h>
#include <mapmem.h>
#include <display_options.h>

/*
 * TODO(sjg@chromium.org): IMO this code should be
 * refactored to a single function, something like:
 *
 * void led_set_state(enum led_colour_t colour, int on);
 */
/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
__weak void coloured_LED_init(void) {}
__weak void red_led_on(void) {}
__weak void red_led_off(void) {}

#define TOTAL_MALLOC_LEN (CONFIG_SYS_MALLOC_LEN + CONFIG_ENV_SIZE)

static int setup_mon_len(void)
{
	gd->mon_len = (ulong)&__bss_end - (ulong)_start;

	return 0;
}

/* Get the top of usable RAM */
__weak ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_RELOC_SYS_TEXT_BASE
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	if (gd->ram_top < CONFIG_RELOC_SYS_TEXT_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		return 0;
#endif
	return gd->ram_top;
}

__weak int board_early_init_f(void)
{
	return 0;
}

__weak int dram_init(void)
{
#ifdef CONFIG_RELOC_SYS_BLOCK_SIZE
	gd->ram_size = CONFIG_RELOC_SYS_BLOCK_SIZE;
#endif
	return 0;
}

__weak phys_size_t get_effective_memsize(void)
{
	return gd->ram_size;
}

#ifdef CONFIG_ARM
__weak int reserve_mmu(void)
{
#ifdef CONFIG_ARMV8_CACHE
	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->relocaddr -= gd->arch.tlb_size;

	/* round down to next 64 kB limit */
	gd->relocaddr &= ~(0x10000 - 1);

	gd->arch.tlb_addr = gd->relocaddr;
	pr_info("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);
#endif
	return 0;
}
#endif

static int setup_dest_addr(void)
{

#ifdef CONFIG_RELOC_SYS_TEXT_BASE
	gd->ram_base = CONFIG_RELOC_SYS_TEXT_BASE;
#endif
	gd->ram_top = gd->ram_base + get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	gd->relocaddr = gd->ram_top;
	pr_info("Monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	pr_info("Ram size: %08lX\n", (ulong)gd->ram_size);
	pr_info("Ram top : %08lX\n", (ulong)gd->ram_top);

	return 0;
}

static int reserve_vmimage(void)
{
	if (!(gd->flags & GD_FLG_SKIP_RELOC)) {
		/*
		 * reserve memory for vmimage code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);

		pr_info("Reserving %ldk for vmimage at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	return 0;
}

static int reserve_global_data(void)
{
	gd->start_addr_sp -= sizeof(gd_t);
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, sizeof(gd_t));
	pr_info("Reserving %zu Bytes for Global Data at: %08lx\n",
	      sizeof(gd_t), gd->start_addr_sp);
	return 0;
}

static int reserve_malloc(void)
{
	unsigned long size;

	/* align 4k */
	gd->start_addr_sp = rounddown(gd->start_addr_sp, 4096);

	if (gd->start_addr_sp - CONFIG_MALLOC_F_LEN - CONFIG_START_ADDR_SP_SIZE
				< gd->ram_base) {
		size = gd->start_addr_sp - gd->ram_base - CONFIG_START_ADDR_SP_SIZE;
		size = rounddown(size, 4096);
	} else
		size = rounddown(CONFIG_MALLOC_F_LEN, 4096);
	printf("%s:%d size:%#x start_addr_sp:%#lx\n", __FUNCTION__, __LINE__,
	       size, gd->start_addr_sp);
	gd->start_addr_sp -= size;
	gd->malloc_base = gd->start_addr_sp;
	gd->malloc_limit = size;
	pr_info("Reserving %ldk for malloc at: %08lx\n",
		    gd->malloc_limit >> 10, gd->malloc_base);
	return 0;
}

__weak int arch_reserve_stacks(void)
{
	return 0;
}

__weak void board_add_ram_info(int use_default)
{
	/* please define platform specific board_add_ram_info() */
}

static int show_dram_config(void)
{
	unsigned long long size;

#ifdef CONFIG_NR_DRAM_BANKS
	int i;

	pr_info("\nRAM Configuration:\n");
	for (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd.bi_dram[i].size;
		pr_info("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->bd.bi_dram[i].start));
		print_size(gd->bd.bi_dram[i].size, "\n");
	}
	pr_info("DRAM:  ");
#else
	size = gd->ram_size;
#endif

	print_size(size, "");
	board_add_ram_info(0);
	puts("\n\n");

	return 0;
}

__weak int dram_init_banksize(void)
{
#ifdef CONFIG_SYSMEM_FLASH_START
	gd->bd.bi_flashstart = CONFIG_SYSMEM_FLASH_START;
	gd->bd.bi_flashsize = CONFIG_SYSMEM_FLASH_SIZE;
#endif
#ifdef CONFIG_SYSMEM_SRAM_START
	gd->bd.bi_sramstart = CONFIG_SYSMEM_SRAM_START;
	gd->bd.bi_sramsize = CONFIG_SYSMEM_SRAM_SIZE;
#endif
#ifdef CONFIG_SYSMEM_DDR0_START
	gd->bd.bi_dram[0].start = CONFIG_SYSMEM_DDR0_START;
	gd->bd.bi_dram[0].size = CONFIG_SYSMEM_DDR0_SIZE;
#endif
#if CONFIG_SYSMEM_DDR1_START
	gd->bd.bi_dram[1].start = CONFIG_SYSMEM_DDR1_START;
	gd->bd.bi_dram[1].size = CONFIG_SYSMEM_DDR1_SIZE;
#endif
	return 0;
}

static int reserve_stacks(void)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp -= 16;
	gd->start_addr_sp &= ~0xf;

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	return arch_reserve_stacks();
}

static int display_new_sp(void)
{
	pr_info("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	return 0;
}

static int setup_reloc(void)
{
	if (gd->flags & GD_FLG_SKIP_RELOC) {
		pr_info("Skipping relocation due to flag\n");
		return 0;
	}

	gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));

	pr_info("Relocation Offset is: %08lx\n", gd->reloc_off);
	pr_info("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
	      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
	      gd->start_addr_sp);

	return 0;
}

static const init_fnc_t init_sequence_f[] = {
	setup_mon_len,
	display_options,
	board_early_init_f,
	dram_init,
	setup_dest_addr,
	reserve_mmu,
	reserve_vmimage,
	reserve_global_data,
	reserve_malloc,
	reserve_stacks,
	dram_init_banksize,
	show_dram_config,
	display_new_sp,
	setup_reloc,
	NULL,
};


__weak void uart_init(int port, int baudrate)
{
}

__weak void timer_init(void)
{
}


void board_init_f(ulong boot_flags)
{
	gd->flags = boot_flags;
	gd->baudrate = CONFIG_PRINT_UART_BAUD;


	// set_sctlr(get_sctlr() & ~(CR_SA | CR_A));
	timer_init();
    uart_init(CONFIG_UART_PORT, CONFIG_PRINT_UART_BAUD);

#ifdef CONFIG_A78_MISC_TEST
		if (readl_relaxed(CONFIG_SYSMEM_DDR0_START + RESET_A78_FLAG_ADDR) == ENTER_RESET_STATUS_MAGIC) {
			a78_cpu_reset_test();
		}
#endif

#ifdef CONFIG_BOOT_TEST
	extern void boot_a78(void);
	boot_a78();
#endif

	if (initcall_run_list(init_sequence_f))
		hang();
}

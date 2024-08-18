// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <irq_func.h>
#include <linux/compiler.h>
#include <asm/ptrace.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int interrupt_init(void)
{
	return 0;
}

__weak void enable_interrupts(void)
{
	return;
}

__weak int disable_interrupts(void)
{
	return 0;
}

static void dump_instr(struct pt_regs *regs)
{
	u32 *addr = (u32 *)(regs->elr & ~3UL);
	int i;

	printf("Code: ");
	for (i = -4; i < 1; i++)
		printf(i == 0 ? "(%08x) " : "%08x ", addr[i]);
	printf("\n");
}

void show_regs(struct pt_regs *regs)
{
	int i;

	if (gd->flags & GD_FLG_RELOC)
		printf("elr: %016lx lr : %016lx (reloc)\n",
		       regs->elr - gd->reloc_off,
		       regs->regs[30] - gd->reloc_off);
	printf("elr: %016lx lr : %016lx\n", regs->elr, regs->regs[30]);

	for (i = 0; i < 29; i += 2)
		printf("x%-2d: %016lx x%-2d: %016lx\n",
		       i, regs->regs[i], i+1, regs->regs[i+1]);
	printf("\n");
	dump_instr(regs);
}

/*
 * do_bad_sync handles the impossible case in the Synchronous Abort vector.
 */
void do_bad_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("Bad mode in \"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("Bad mode in \"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("Bad mode in \"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("Bad mode in \"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

static u64 get_pa_fault(void)
{
	u32 el = current_el();
	u64 addr = 0;
	asm volatile("dsb sy");
	if (el == 1) {
		asm volatile("mrs %0, far_el1" : "=r" (addr):  : "memory");

	} else if (el == 2) {
		asm volatile("mrs %0, far_el2" : "=r" (addr):  : "memory");
	} else if (el == 3) {
		asm volatile("mrs %0, far_el3" : "=r" (addr):  : "memory");
	} else {
		;
	}
	asm volatile("isb");
	pr_debug("\n el:%d addr:0x%llx\n", el, addr);
	return addr;
}

static u64 get_ipa_fault(void)
{
	u32 el = current_el();
	u64 addr = 0;
	asm volatile("dsb sy");
	if (el == 2) {
		asm volatile("mrs %0, hpfar_el2" : "=r" (addr):  : "memory");

	} 
	asm volatile("isb");
	return addr;
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("\"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	printf("\n IPA:0x%llx VA:0x%llx\n", get_ipa_fault(), get_pa_fault());
	panic("Resetting CPU ...\n");
}

/*
 * do_irq handles the Irq exception.
 */
__weak void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("\"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_fiq handles the Fiq exception.
 */
__weak void do_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("\"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct pt_regs *pt_regs, unsigned int esr)
{
	printf("\"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Header file for interrupt functions
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __IRQ_FUNC_H
#define __IRQ_FUNC_H

#define LPI_IRQ_MIN (8192)
#define MIN_IRQ_ID 25
#ifdef CONFIG_GICV3_TEST
	#ifdef CONFIG_GICV3_ITS
	#define MAX_IRQ_ID (LPI_IRQ_MIN + 32)
	#else
	#define MAX_IRQ_ID 1024
	#endif
#else
#define MAX_IRQ_ID 260
#endif

#define IRQ_TYPE_NONE (0x00000000)
#define IRQ_TYPE_EDGE_RISING (0x00000001)
#define IRQ_TYPE_EDGE_FALLING (0x00000002)
#define IRQ_TYPE_EDGE_BOTH (IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING)
#define IRQ_TYPE_LEVEL_HIGH (0x00000004)
#define IRQ_TYPE_LEVEL_LOW (0x00000008)
#define IRQ_TYPE_LEVEL_MASK (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH)

/*
 * irq_get_irqchip_state/irq_set_irqchip_state specific flags
 */
enum irqchip_irq_state {
	IRQCHIP_STATE_PENDING,		/* Is interrupt pending? */
	IRQCHIP_STATE_ACTIVE,		/* Is interrupt in progress? */
	IRQCHIP_STATE_MASKED,		/* Is interrupt masked? */
	IRQCHIP_STATE_LINE_LEVEL,	/* Is IRQ line high? */
};

/* define flag */
/* which cpu */
#define ISR_ATTR_SECURE 0x1
#define ISR_ATTR_SAFETY 0x2
#define ISR_ATTR_A55 0x4
#define ISR_ATTR_GEN 0x4

/* level or edge( bit 16) */
#define ISR_ATTR_LEVEL 0X00000
#define ISR_ATTR_EDGE 0X20000
#define IS_EDGE(a) ((a & 0x30000) >> 16)

struct pt_regs;

typedef void (interrupt_handler_t)(void *arg);

int interrupt_init(void);
void timer_interrupt(struct pt_regs *regs);
void external_interrupt(struct pt_regs *regs);
int irq_install_handler(int irq, interrupt_handler_t *handler, void *arg, u32 flag);
void irq_free_handler(int irq);
void reset_timer(void);

void enable_interrupts(void);
int disable_interrupts(void);

void enable_irq(int irq);
void disable_irq(int irq);

int irq_set_irqchip_state(int irq, enum irqchip_irq_state which, bool val);
int irq_get_irqchip_state(int irq, enum irqchip_irq_state which, bool *val);

int irq_set_type(int irq, unsigned int type);

void irq_eoi(int irq);

void irq_unmask(int irq);
void irq_mask(int irq);

extern void gic_init(void);
void gicr_init(void);

struct irq_handler_struct {
    u32 irqid;
    interrupt_handler_t *handler;
    void *arg;
    char *irqname;
    int irqflags;
};

extern struct irq_handler_struct irq_handler_tab[MAX_IRQ_ID + 1];

#define IRQ_DEFINE(__IrqId, __IrqHandle, __arg, __IrqName, __IrqFlags)        \
	__attribute__((section(".section_irq_table"))) struct irq_handler_struct   \
		_section_item_##__IrqId##_tlb = {                              \
			.irqid = __IrqId,                                      \
			.handler = __IrqHandle,                            \
			.arg = __arg,                                        \
			.irqname = __IrqName,                                  \
			.irqflags = __IrqFlags,                                \
		}

#endif

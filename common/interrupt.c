#include <common.h>
#include <cli.h>
#include <command.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/system.h>

extern unsigned long __section_irq_table_start;
extern unsigned long __section_irq_table_end;

struct irq_handler_struct irq_handler_tab[MAX_IRQ_ID + 1] = {0};

static void gic_register_irq(u32 irq, interrupt_handler_t *handler, u32 flags, void *arg, char *name)
{
    if (irq >= MAX_IRQ_ID) {
		pr_err("%s-> irq id is not right.\n", __func__);
		return ;
	}


    pr_debug("%s-> irq:%d reged.\n", __func__, irq);
	irq_handler_tab[irq].irqflags = flags;
	irq_handler_tab[irq].handler = handler;
	irq_handler_tab[irq].irqid = irq;
	irq_handler_tab[irq].irqname = name;
	irq_handler_tab[irq].arg = arg;

	/*  LPI IRQ enabled by GICR in LPI table */
	if (irq < LPI_IRQ_MIN) {
		enable_irq(irq);
	}
}

static void gic_unregister_irq(u32 irq)
{
    if (irq >= MAX_IRQ_ID) {
		pr_err("%s-> irq id is not right.\n", __func__);
		return ;
	}

	irq_handler_tab[irq].handler = NULL;
    disable_irq(irq);
}

static void irq_table_init(void)
{
    struct irq_handler_struct *irq_tab = (struct irq_handler_struct *)&__section_irq_table_start;
    int irqid;

    for (; irq_tab < (struct irq_handler_struct *)&__section_irq_table_end; irq_tab++) {
        irqid = irq_tab->irqid;

        if (ISR_ATTR_GEN & irq_tab->irqflags)
            gic_register_irq(irq_tab->irqid, irq_tab->handler,
                    irq_tab->irqflags, irq_tab->arg, irq_tab->irqname);
    }
}

int interrupt_init(void)
{
	local_irq_disable();
#if defined(CONFIG_GICV3_TEST) || defined(CONFIG_GICV2_TEST)
	gic_init();
	irq_table_init();
#endif
	local_irq_enable();

    return 0;
}

int irq_install_handler(int irq, interrupt_handler_t *handler, void *arg, u32 flag)
{
    gic_register_irq(irq, handler, flag, arg, "soft install");
    return 0;
}

void irq_free_handler(int irq)
{
	int irqid;
	struct irq_handler_struct *irq_tab = (struct irq_handler_struct *)&__section_irq_table_start;

	gic_unregister_irq(irq);

    for (; irq_tab < (struct irq_handler_struct *)&__section_irq_table_end; irq_tab++) {
		irqid = irq_tab->irqid;

		if (irq != irqid)
			continue;

        if (ISR_ATTR_A55 & irq_tab->irqflags)
            gic_register_irq(irq_tab->irqid, irq_tab->handler,
                    irq_tab->irqflags, irq_tab->arg, irq_tab->irqname);
    }
}

void enable_interrupts(void)
{
    // local_irq_enable();
}

int disable_interrupts(void)
{
    // local_irq_disable();

    return 0;
}

// extern void switch_to_el3_g(void);
extern void switch_to_el2_g(void);
extern void switch_to_el1_g(void);
extern void switch_to_el0_g(void);

static int swtich_to_el3_test(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned int el_stat;
	dcache_disable();
	// switch_to_el3_g();
	el_stat = current_el();
	printf("Now In EL%d\n\r", el_stat);

	return CMD_RET_SUCCESS;
}
static int swtich_to_el2_test(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned int el_stat;
	dcache_disable();
	switch_to_el2_g();
	el_stat = current_el();
	printf("Now In EL%d\n\r", el_stat);

	return CMD_RET_SUCCESS;
}

static int swtich_to_el1_test(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned int el_stat;

	switch_to_el1_g();
	el_stat = current_el();
	printf("Now In EL%d\n\r", el_stat);
	return CMD_RET_SUCCESS;
}

static int swtich_to_el0_test(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned int el_stat = 0;

	switch_to_el0_g();

	printf("Now In EL%d\n\r", el_stat);
	return CMD_RET_SUCCESS;
}
static int get_current_el(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned int el_stat = 0;
	el_stat = current_el();
	printf("Now In EL%d\n\r", el_stat);
	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_switch_sub[] = {
	ROCK_CMD_MKENT(el3, 1, 1, swtich_to_el3_test, "", ""),
	ROCK_CMD_MKENT(el2, 1, 1, swtich_to_el2_test, "", ""),
	ROCK_CMD_MKENT(el1, 3, 1, swtich_to_el1_test, "", ""),
	ROCK_CMD_MKENT(el0, 3, 1, swtich_to_el0_test, "", ""),
	ROCK_CMD_MKENT(get, 3, 1, get_current_el, "", ""),
};

static int do_switch_el(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_switch_sub[0], ARRAY_SIZE(cmd_switch_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

ROCK_CMD(switch_el, CONFIG_SYS_MAXARGS, 0, do_switch_el,
	   "armv8 switch el test",
	   "switch_el el2\n\r"
       "switch_el el1\n\r"
       "switch_el get\n\r"
       "switch_el el0");

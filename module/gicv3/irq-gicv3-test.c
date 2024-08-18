#include <common.h>
#include <sysreg_base.h>
#include <asm/sysreg.h>

#include <asm/system.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <time.h>
#include <public.h>

#include "arm-gic-v3.h"
#include "irq-gic-common.h"
#include <module/gicv3.h>
#include <asm/cputype.h>
#include "gicv3_its.h"

#define PRINT_SYS_REG(_reg) printf(#_reg " 0x%0x\n", (u32)read_sysreg_s(_reg))

#define MIN_IRQ_ID 25
#define MAcpuX_IRQ_ID 260
#define IRQ_PPI_MIN IRQ_PPI_GIC_MAIN_INT
#define SPI_NUM (36)
#define WDT_CR 0x0
#define WDT_TORR 0x4
#define DW_TIMER_MIN_PERIOD             4
#define DW_TIMER_MIN_DELTA_USEC         200

#define DW_TIMER_N_LOAD_COUNT           0x00
#define DW_TIMER_N_CURRENT_VALUE        0x04
#define DW_TIMER_N_CONTROL              0x08
#define DW_TIMER_N_EOI                  0x0c
#define DW_TIMER_N_INT_STATUS           0x10

#define DW_TIMER_INT_STATUS             0xa0
#define DW_TIMER_EOI                    0xa4
#define DW_TIMER_RAW_INT_STATUS         0xa8
#define DW_TIMER_COMP_VERSION           0xac

#define DW_TIMER_CONTROL_ENABLE         (1 << 0)
/* 1: periodic, 0:free running. */
#define DW_TIMER_CONTROL_MODE_PERIODIC  (1 << 1)
#define DW_TIMER_CONTROL_INT            (1 << 2)
#define RESULT_IGNORE 2

static int irq_handler_test_tab[MAX_IRQ_ID + 1] = { 0 };
static volatile int irq_test_curr_pos = 0;
static int irq_test_prio[32] = { -1 };
extern struct irq_gic_v3 gic_data;
extern void switch_to_el1_g(void);

#define GICV600AE_MAX_INTR (gic_data.gic_irqs)


static int gicv3_register_dump_test(void)
{
	int i;
	void *dist_base = gic_data.dist_base;
	void *rd_sgi_base = gic_data.rdist_base + GICR_SGI_BASE;

	printf("== Dump GIC CPU registers. ==\n");
	PRINT_SYS_REG(SYS_ICC_CTLR_EL1);
	PRINT_SYS_REG(SYS_ICC_PMR_EL1);
	PRINT_SYS_REG(SYS_ICC_BPR0_EL1);
	PRINT_SYS_REG(SYS_ICC_BPR1_EL1);
	PRINT_SYS_REG(SYS_ICC_IAR0_EL1);
	PRINT_SYS_REG(SYS_ICC_IAR1_EL1);
	PRINT_SYS_REG(SYS_ICC_IGRPEN0_EL1);
	PRINT_SYS_REG(SYS_ICC_IGRPEN1_EL1);
	// PRINT_SYS_REG(SYS_ICC_IGRPEN1_EL3);

	PRINT_SYS_REG(SYS_ICC_RPR_EL1);
	PRINT_SYS_REG(SYS_ICC_HPPIR0_EL1);
	PRINT_SYS_REG(SYS_ICC_HPPIR1_EL1);
	printf("== Dump GIC Distributor registers. ==\n");
	printf("GICD_CTLR  :0x%0x\n", readl(dist_base + GICD_CTLR));
	printf("GICD_TYPER :0x%0x\n", readl(dist_base + GICD_TYPER));
	printf("GICD_IIDR  :0x%0x\n", readl(dist_base + GICD_IIDR));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_IGROUPRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_IGROUPR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ISENABLERn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ISENABLER + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ICENABLERn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ICENABLER + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ISPENDRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ISPENDR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ICPENDRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ICPENDR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ISACTIVERn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ISACTIVER + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ICACTIVERn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ICACTIVER + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_IPRIORITYRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_IPRIORITYR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ITARGETSRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ITARGETSR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_ICFGR i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_ICFGR + i));

	printf("GICD_SGIR i: %d val: 0x%x\n", i,
	       readl(dist_base + GICD_SGIR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_CPENDSGIRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_CPENDSGIR + i));

	for (i = 0; i < GICV600AE_MAX_INTR / 8; i += 4)
		printf("GICD_SPENDSGIRn i: %d val: 0x%x\n", i,
		       readl(dist_base + GICD_SPENDSGIR + i));
	printf("== Dump cpu0 GIC Rdistributor registers. ==\n");
	printf("GICR_IGROUPR0 0x%0x\n", readl(rd_sgi_base + GICR_IGROUPR0));
	printf("GICR_IGRPMODR0 0x%0x\n", readl(rd_sgi_base + GICR_IGRPMODR0));

	printf("GICR_ICACTIVER0 0x%0x\n", readl(rd_sgi_base + GICR_ICACTIVER0));
	printf("GICR_ICENABLER0 0x%0x\n", readl(rd_sgi_base + GICR_ICENABLER0));
	printf("GICR_ISENABLER0 0x%0x\n", readl(rd_sgi_base + GICR_ISENABLER0));
	for (i = 0; i < 32; i += 4) {
		printf("GICR_IPRIORITYR[%d] 0x%0x\n", i, readl(rd_sgi_base + GICR_IPRIORITYR0 + i));
	}
	return 0;
}

int module_gicv3_test(int case_id, unsigned long *arg_list, int argc)
{
	int ret = 0;

	printf("case: %d\n", case_id);

	switch (case_id) {
	case 0:
		ret = gicv3_register_dump_test();
		break;
	case 1:
		local_irq_disable();
		break;
	default:
		printf("Unkonw case.\n");

		ret = 1;
		break;
	}

	return ret;
}

void module_gicv3_test_help(void)
{
	printf("gicv3 test 0       - Gicv3 Register Dump\n");
}


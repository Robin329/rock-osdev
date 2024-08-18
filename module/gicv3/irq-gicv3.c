#define pr_fmt(fmt) KBUILD_BASENAME ": " fmt

#include <common.h>
#include <sysreg_base.h>

#include <asm/sysreg.h>
#include <asm/system.h>
#include <asm/cputype.h>

#include "irq-gic-common.h"
#include "arm-gic-v3.h"

struct irq_gic_v3 gic_data;

#define CPU_NUM 1
#define MPIDR_RS(mpidr) (((mpidr) & 0xF0UL) >> 4)
/* Our default, arbitrary priority value. Linux only uses one anyway. */
#define DEFAULT_PMR_VALUE 0xff
static void gicv3_sysreg_support(void)
{
	unsigned long value;

	asm volatile("mrs 	%0, id_aa64pfr0_el1\n\t"
		     "ubfx	%0, %0, #24, #4\n\t"
		     : "=r"(value)::);

	if (value) {
		value = read_sysreg_s(SYS_ICC_SRE_EL3);
		value |= ICC_SRE_EL3_SRE;
		value |= ICC_SRE_EL3_ENABLE;
		value |= (1 << 1) | (1 << 2);
		write_sysreg_s(value, SYS_ICC_SRE_EL3);
		isb();
		write_sysreg_s(value, SYS_ICC_SRE_EL2);
		isb();
		value = read_sysreg_s(SYS_ICC_SRE_EL3);
		if (value)
			pr_info("Cpu support Gicv3 SRE:%x\n", value);
		else
			write_sysreg_s(0, SYS_ICH_HCR_EL2);
	}
}

static int gic_validate_dist_version(void *dist_base)
{
	u32 reg = readl_relaxed(dist_base + GICD_PIDR2) & GIC_PIDR2_ARCH_MASK;

	if (reg != GIC_PIDR2_ARCH_GICv3 && reg != GIC_PIDR2_ARCH_GICv4)
		return -ENODEV;

	return 0;
}

static int gic_iterate_rdists(int (*fn)(void *))
{
	int ret, count;
	u32 reg;
	u64 typer;
	void *ptr = gic_data.rdist_base;

	reg = readl_relaxed(ptr + GICR_PIDR2) & GIC_PIDR2_ARCH_MASK;
	if (reg != GIC_PIDR2_ARCH_GICv3 && reg != GIC_PIDR2_ARCH_GICv4) {
		pr_warn("No redistributor present @%p\n", ptr);
		return -ENODEV;
	}

	count = 0;
	do {
		typer = gic_read_typer(ptr + GICR_TYPER);
		pr_info("CPU%d: GICR_TYPER:0x%lx \n", count, typer);

		ret = fn(ptr);
		if (!ret)
			return 0;

		ptr += SZ_64K * 2; /* Skip RD_base + SGI_base */
		if (typer & GICR_TYPER_VLPIS)
			ptr += SZ_64K * 2; /* Skip VLPI_base + reserved page */
		count++;
	} while (!(typer & GICR_TYPER_LAST));

	if (count != CPU_NUM)
		pr_warn("\"WARN\"no Redistributor present to %d core.!\n",
			CPU_NUM);

	return 0;
}
static int __gic_update_vlpi_properties(void *ptr)
{
	u64 typer = gic_read_typer(ptr + GICR_TYPER);
	gic_data.has_vlpis &= !!(typer & GICR_TYPER_VLPIS);
	gic_data.has_direct_lpi &= !!(typer & GICR_TYPER_DirectLPIS);

	return 1;
}

static void gic_update_vlpi_properties(void)
{
	gic_iterate_rdists(__gic_update_vlpi_properties);
	pr_info("%sVLPI support, %sdirect LPI support\n",
		!gic_data.has_vlpis ? "no " : "",
		!gic_data.has_direct_lpi ? "no " : "");
}

static void gic_do_wait_for_rwp(void *base)
{
	u32 count = 1000000; /* 1s! */
	while (readl_relaxed(base + GICD_CTLR) & GICD_CTLR_RWP) {
		count--;
		if (!count) {
			pr_err("ERROR: RWP timeout, gone fishing\n");
			return;
		}
		cpu_relax();
		udelay(1);
	};
}

/* Wait for completion of a distributor change */
static void gic_dist_wait_for_rwp(void)
{
	gic_do_wait_for_rwp(gic_data.dist_base);
}

static u64 gic_mpidr_to_affinity(unsigned long mpidr)
{
	u64 aff;

	aff = ((u64)MPIDR_AFFINITY_LEVEL(mpidr, 3) << 32 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 0));

	return aff;
}

static void gic_dist_init(void)
{
	unsigned int i;
	u64 affinity, mpidr;
	void *base = gic_data.dist_base;

	/* Disable the distributor */
	writel_relaxed(0, base + GICD_CTLR);
	gic_dist_wait_for_rwp();

	/*
	 * Configure SPIs as non-secure Group-1. This will only matter
	 * if the GIC only has a single security state. This will not
	 * do the right thing if the kernel is running in secure mode,
	 * but that's not the intended use case anyway.
	 * (32 - gic_irqs) configure to non-secure Group-1 irq.
	 */
	for (i = 32; i < gic_data.gic_irqs; i += 32)
		writel_relaxed(~0, base + GICD_IGROUPR + i / 8);

	gic_dist_config(base, gic_data.gic_irqs, gic_dist_wait_for_rwp);

	/* enable non-secure access gic */
	writel_relaxed(readl_relaxed(base + GICD_CTLR) | GICD_CTLR_DS,
		       base + GICD_CTLR);
	gic_dist_wait_for_rwp();

	/* Enable distributor with ARE, Group1 */
	writel_relaxed((1 << 5) | (1 << 1) | GICD_CTLR_ENABLE_G1,
		       base + GICD_CTLR);
	/*
	 * Set all global interrupts to the boot CPU only. ARE must be
	 * enabled.
	 */
	mpidr = read_cpuid_mpidr() & MPIDR_HWID_BITMASK;
	affinity = gic_mpidr_to_affinity(mpidr);
	for (i = 32; i < gic_data.gic_irqs; i++)
		gic_write_irouter(affinity, base + GICD_IROUTER + i * 8);
}

static int __gic_populate_rdist(void *ptr)
{
	unsigned long mpidr = read_cpuid_mpidr() & MPIDR_HWID_BITMASK;
	u64 typer;
	u32 aff;

	/*
	 * Convert affinity to a 32bit value that can be matched to
	 * GICR_TYPER bits [63:32].
	 */
	aff = (MPIDR_AFFINITY_LEVEL(mpidr, 3) << 24 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 0));

	typer = gic_read_typer(ptr + GICR_TYPER);
	if ((typer >> 32) == aff) {
		pr_info("CPU0: found redistributor %lx region %p\n", mpidr,
			ptr);
		return 0;
	}

	/* Try next one */
	return 1;
}
static int gic_populate_rdist(void)
{
	if (gic_iterate_rdists(__gic_populate_rdist) == 0)
		return 0;

	/* We couldn't even deal with ourselves... */
	pr_warn("CPU0: mpidr %lx has no re-distributor!\n",
		(unsigned long)(read_cpuid_mpidr() & MPIDR_HWID_BITMASK));
	return -ENODEV;
}

static void gic_do_wait_for_redist_rwp(void *base)
{
	u32 count = 1000000; /* 1s! */
	while (readl_relaxed(base + GICR_CTLR) & GICR_CTLR_RWP) {
		count--;
		if (!count) {
			pr_err("ERROR: RDIST RWP timeout, gone fishing\n");
			return;
		}
		cpu_relax();
		udelay(1);
	};
}

/* Wait for completion of a redistributor change */
static void gic_redist_wait_for_rwp(void)
{
	gic_do_wait_for_redist_rwp(gic_data.rdist_base);
}
static void gic_enable_redist(bool enable)
{
	void *rbase;
	u32 count = 1000000; /* 1s! */
	u32 val;

	rbase = gic_data.rdist_base;

	val = readl_relaxed(rbase + GICR_PWRR);
	val &= ~0x1;
	writel_relaxed(val, rbase + GICR_PWRR);

	gic_redist_wait_for_rwp();

	val = readl_relaxed(rbase + GICR_WAKER);
	if (enable)
		/* Wake up this CPU redistributor */
		val &= ~GICR_WAKER_ProcessorSleep;
	else
		val |= GICR_WAKER_ProcessorSleep;
	writel_relaxed(val, rbase + GICR_WAKER);

	if (!enable) { /* Check that GICR_WAKER is writeable */
		val = readl_relaxed(rbase + GICR_WAKER);
		if (!(val & GICR_WAKER_ProcessorSleep))
			return; /* No PM support in this redistributor */
	}

	while (--count) {
		val = readl_relaxed(rbase + GICR_WAKER);
		if (enable ^ (bool)(val & GICR_WAKER_ChildrenAsleep))
			break;
		cpu_relax();
		udelay(1);
	};
	if (!count)
		pr_err("redistributor failed to %s...\n",
		       enable ? "wakeup" : "sleep");
}

static void gic_cpu_sys_reg_init(void)
{
	bool group0;
	u32 val, pribits;

	/*
	 * Need to check that the SRE bit has actually been set. If
	 * not, it means that SRE is disabled at EL2. We're going to
	 * die painfully, and there is nothing we can do about it.
	 *
	 * Kindly inform the luser.
	 */
	if (!gic_enable_sre())
		panic("GIC: unable to set SRE (disabled at EL2), panic ahead\n");

	pribits = gic_read_ctlr();
	pribits &= ICC_CTLR_EL1_PRI_BITS_MASK;
	pribits >>= ICC_CTLR_EL1_PRI_BITS_SHIFT;
	pribits++;

	/*
	 * Let's find out if Group0 is under control of EL3 or not by
	 * setting the highest possible, non-zero priority in PMR.
	 *
	 * If SCR_EL3.FIQ is set, the priority gets shifted down in
	 * order for the CPU interface to set bit 7, and keep the
	 * actual priority in the non-secure range. In the process, it
	 * looses the least significant bit and the actual priority
	 * becomes 0x80. Reading it back returns 0, indicating that
	 * we're don't have access to Group0.
	 */
	write_gicreg(BIT(8 - pribits), ICC_PMR_EL1);
	val = read_gicreg(ICC_PMR_EL1);
	group0 = val != 0;

	/* Set priority mask register */
	write_gicreg(DEFAULT_PMR_VALUE, ICC_PMR_EL1);

	/*
	 * Some firmwares hand over to the kernel with the BPR changed from
	 * its reset value (and with a value large enough to prevent
	 * any pre-emptive interrupts from working at all). Writing a zero
	 * to BPR restores is reset value.
	 */
	gic_write_bpr1(0);

	/* EOI drops priority only (mode 1) */
	gic_write_ctlr(ICC_CTLR_EL1_EOImode_drop);

	/* Always whack Group0 before Group1 */
	if (group0) {
		switch (pribits) {
		case 8:
		case 7:
			write_gicreg(0, ICC_AP0R3_EL1);
			write_gicreg(0, ICC_AP0R2_EL1);
		case 6:
			write_gicreg(0, ICC_AP0R1_EL1);
		case 5:
		case 4:
			write_gicreg(0, ICC_AP0R0_EL1);
		}

		isb();
	}

	switch (pribits) {
	case 8:
	case 7:
		write_gicreg(0, ICC_AP1R3_EL1);
		write_gicreg(0, ICC_AP1R2_EL1);
	case 6:
		write_gicreg(0, ICC_AP1R1_EL1);
	case 5:
	case 4:
		write_gicreg(0, ICC_AP1R0_EL1);
	}

	isb();

	/* ... and let's hit the road... */
	gic_write_grpen1(1);
}
static void gic_cpu_init(void)
{
	void *rd_sgi_base = gic_data.rdist_base + GICR_SGI_BASE;

	/* Register ourselves with the rest of the world */
	if (gic_populate_rdist())
		return;

	gic_enable_redist(true);

	/* Configure SGIs/PPIs as non-secure Group-1 */
	writel_relaxed(~0, rd_sgi_base + GICR_IGROUPR0);
	writel_relaxed(0, rd_sgi_base + GICR_IGRPMODR0);

	write_sysreg_s(ICC_IGRPEN0_EL1_MASK, SYS_ICC_IGRPEN0_EL1);
	write_sysreg_s(ICC_IGRPEN1_EL1_MASK, SYS_ICC_IGRPEN1_EL1);

	// write_sysreg_s((0x1 << 0), SYS_ICC_IGRPEN1_EL3);


	write_sysreg_s(0xff, SYS_ICC_PMR_EL1);

	isb();

	gic_cpu_config(rd_sgi_base, gic_redist_wait_for_rwp);

	/* initialise system registers */
	gic_cpu_sys_reg_init();

}

static int gic_dist_supports_lpis(void)
{
	return !!(readl_relaxed(gic_data.dist_base + GICD_TYPER) &
		  GICD_TYPER_LPIS);
}
static int gic_init_bases(void *dist_base, void *rdist_base)
{
	u32 typer;
	int gic_irqs;

	typer = readl_relaxed(dist_base + GICD_TYPER);
	gic_irqs = GICD_TYPER_IRQS(typer);
	if (gic_irqs > 1020)
		gic_irqs = 1020;
	gic_data.gic_irqs = gic_irqs;
	gic_data.has_rss = !!(typer & GICD_TYPER_RSS);
	gic_data.typer = typer;
	gic_data.has_vlpis = true;
	gic_data.has_direct_lpi = true;
	pr_info("Distributor has %sRange Selector support, Support %d irq\n",
		gic_data.has_rss ? "" : "no ", gic_irqs);

	if (typer & GICD_TYPER_MBIS)
		pr_info("Gicv3 support MBIs, TODO MSI!\n");

	// gic_update_vlpi_properties();

	gic_dist_init();

	gic_cpu_init();


	if (gic_dist_supports_lpis()) {
		pr_info("Support LPIs,TODO!\n");
	}

	return 0;
}

void gic_init(void)
{
	int ret;
	void *dist_base = (void *)GICV3_DIST_BASE;
	void *rdist_base = (void *)GICV3_RDIST_0_BASE;

	gic_data.dist_base = dist_base;
	gic_data.rdist_base = rdist_base;

	// gicv3_sysreg_support();

	ret = gic_validate_dist_version(dist_base);
	if (ret)
		panic("Gicv3: no distributor detected, giving up!\n");

	ret = gic_init_bases(dist_base, rdist_base);
	if (ret)
		panic("Gicv3: Init gic bases failed! (%d)\n", ret);
}
static u32 gic_get_cpuid(void)
{
	u32 mpidr = (u64)read_sysreg_s(SYS_MPIDR_EL1);
	u32 affinity2 = (mpidr >> 16) & 0xff;
	u32 cpu_id;

	cpu_id = (4 * affinity2) + ((mpidr >> 8) & 0xff);

	return cpu_id;
}
#define gic_data_rdist_rd_base()                                               \
	(gic_data.rdist_base + (gic_get_cpuid() * (SZ_128K)))
#define gic_data_rdist_sgi_base() (gic_data_rdist_rd_base() + SZ_64K)

static inline int gic_irq_in_rdist(int irq)
{
	return irq < 32;
}

/*
 * Routines to disable, enable, EOI and route interrupts
 */
static int gic_peek_irq(int irq, u32 offset)
{
	u32 mask = 1 << (irq % 32);
	void __iomem *base;

	if (gic_irq_in_rdist(irq))
		base = gic_data_rdist_sgi_base();
	else
		base = gic_data.dist_base;

	return !!(readl_relaxed(base + offset + (irq / 32) * 4) & mask);
}


static void gic_poke_irq(int irq, u32 offset)
{
	u32 mask = 1 << (irq % 32);
	void (*rwp_wait)(void);
	void *base;

	if (gic_irq_in_rdist(irq)) {
		base = gic_data_rdist_sgi_base();
		rwp_wait = gic_redist_wait_for_rwp;
		//pr_info("set gicr_dist\n");
		udelay(5);
	} else {
		base = gic_data.dist_base;
		rwp_wait = gic_dist_wait_for_rwp;
	}

	writel_relaxed(mask, base + offset + (irq / 32) * 4);
	rwp_wait();
}
static void gic_mask_irq(int irq)
{
	gic_poke_irq(irq, GICD_ICENABLER);
}

static void gic_unmask_irq(int irq)
{
	gic_poke_irq(irq, GICD_ISENABLER);
}

static void gic_eoimode1_mask_irq(int irq)
{
	gic_mask_irq(irq);
}

/*
 * General function
 */
void irq_mask(int irq)
{
	gic_eoimode1_mask_irq(irq);
}

void irq_unmask(int irq)
{
	gic_unmask_irq(irq);
}

static int gic_irq_set_irqchip_state(int irq, enum irqchip_irq_state which,
				     bool val)
{
	u32 reg;

	if (irq >= gic_data.gic_irqs)
		return -EINVAL;

	switch (which) {
	case IRQCHIP_STATE_PENDING:
		reg = val ? GICD_ISPENDR : GICD_ICPENDR;
		break;
	case IRQCHIP_STATE_ACTIVE:
		reg = val ? GICD_ISACTIVER : GICD_ICACTIVER;
		break;
	case IRQCHIP_STATE_MASKED:
		reg = val ? GICD_ICENABLER : GICD_ISENABLER;
		break;
	default:
		return -EINVAL;
	}

	gic_poke_irq(irq, reg);
	return 0;
}

static void gic_eoimode1_eoi_irq(int irq)
{
	/*
	 * No need to deactivate an LPI, or an interrupt that
	 * is is getting forwarded to a vcpu.
	 */
	if (irq >= 8192) {
		pr_warn("irq %d TODO\n", irq);
		return;
	}

	gic_write_dir(irq);
}
void irq_eoi(int irq)
{
	gic_eoimode1_eoi_irq(irq);
}

static int gic_set_type(int irq, unsigned int type)
{
	void (*rwp_wait)(void);
	void *base;

	/* Interrupt configuration for SGIs can't be changed */
	if (irq < 16)
		return -EINVAL;

	/* SPIs have restrictions on the supported types */
	if (irq >= 32 && type != IRQ_TYPE_LEVEL_HIGH &&
	    type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	if (gic_irq_in_rdist(irq)) {
		base = gic_data_rdist_sgi_base();
		rwp_wait = gic_redist_wait_for_rwp;
	} else {
		base = gic_data.dist_base;
		rwp_wait = gic_dist_wait_for_rwp;
	}

	return gic_configure_irq(irq, type, base, rwp_wait);
}

int irq_set_type(int irq, unsigned int type)
{
	return gic_set_type(irq, type);
}

int gic_irq_get_irqchip_state(int irq, enum irqchip_irq_state which, bool *val)
{
	if (irq >= gic_data.gic_irqs)
		return -EINVAL;

	switch (which) {
	case IRQCHIP_STATE_PENDING:
		*val = gic_peek_irq(irq, GICD_ISPENDR);
		break;
	case IRQCHIP_STATE_ACTIVE:
		*val = gic_peek_irq(irq, GICD_ISACTIVER);
		break;
	case IRQCHIP_STATE_MASKED:
		*val = !gic_peek_irq(irq, GICD_ISENABLER);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
int irq_get_irqchip_state(int irq, enum irqchip_irq_state which, bool *val)
{
	return gic_irq_get_irqchip_state(irq, which, val);
}

int irq_set_irqchip_state(int irq, enum irqchip_irq_state which, bool val)
{
	return gic_irq_set_irqchip_state(irq, which, val);
}

void enable_irq(int irq)
{
	irq_unmask(irq);
}

void disable_irq(int irq)
{
	irq_mask(irq);
}

static void gicv3_clean_active_irq(int irqid)
{
	gic_poke_irq(irqid, GICD_ICACTIVERn);
}

void do_irq_handle(void)
{
	u32 irqstat, irqnr;

	do {
		irqnr = 0;
		//irqstat = gic_read_iar_common();
		irqstat = (u32)read_sysreg_s(SYS_ICC_IAR1_EL1);
		isb();
		irqnr = irqstat & GICC_IAR_INT_MASK;
		//pr_info("do_irq IRQ: %d irqstat:%u Catched.---\n", irqnr, irqstat);
		if ((irqnr < 1020) || (irqnr >= LPI_IRQ_MIN)) {
			gic_write_eoir(irqnr);

			pr_debug("IRQ: %d Catched.\n", irqnr);
			if (irq_handler_tab[irqnr].handler) {
				irq_handler_tab[irqnr].handler(
					irq_handler_tab[irqnr].arg);
			} else {
				printf("%d Not Found handler.please Check IRQ_DEFINE.\n",
				       irqnr);
			}

			if (irqnr < 1020) {
				gicv3_clean_active_irq(irqstat);
			}
			// continue;
		}
		break;
	} while (1);
}

void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	pr_debug("\"Irq\" handler, esr 0x%08x\n", esr);
	do_irq_handle();
	//panic("This irq handler!\n");
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	pr_debug("\"Fiq\" handler, esr 0x%08x\n", esr);
	//panic("This fiq handler!\n");
	do_irq_handle();
}

static void gic_wakeup_redist(bool enable, void *rbase)
{
	u32 count = 1000; /* 1s! */
	u32 val;

	val = readl_relaxed(rbase + GICR_PWRR);
	val &= ~0x1;
	writel_relaxed(val, rbase + GICR_PWRR);

	gic_redist_wait_for_rwp();

	val = readl_relaxed(rbase + GICR_WAKER);
	if (enable)
		/* Wake up this CPU redistributor */
		val &= ~GICR_WAKER_ProcessorSleep;
	else
		val |= GICR_WAKER_ProcessorSleep;
	writel_relaxed(val, rbase + GICR_WAKER);

	if (!enable) { /* Check that GICR_WAKER is writeable */
		val = readl_relaxed(rbase + GICR_WAKER);
		if (!(val & GICR_WAKER_ProcessorSleep))
			return; /* No PM support in this redistributor */
	}

	while (--count) {
		val = readl_relaxed(rbase + GICR_WAKER);
		if (enable ^ (bool)(val & GICR_WAKER_ChildrenAsleep))
			break;
		cpu_relax();
		udelay(1);
	};
	if (!count)
		pr_err("redistributor failed to %s...\n",
		       enable ? "wakeup" : "sleep");
}

static void *get_gicr_sgi_base(void)
{
	u32 cpuid = gic_get_cpuid();

	void *sgi_base =
		gic_data.rdist_base + (cpuid * GICR_OFFSET) + GICR_SGI_BASE;

	return sgi_base;
}

/* Wait for completion of a redistributor change */
static void gicr_wait_for_rwp(void)
{
	gic_do_wait_for_redist_rwp(get_gicr_sgi_base());
}

void gicr_init(void)
{
	u32 cpuid = gic_get_cpuid();
	u32 daif = 0;
	void *gicr_base = gic_data.rdist_base + (cpuid * GICR_OFFSET);
	void *rd_sgi_base = gicr_base + GICR_SGI_BASE;

	asm volatile("mrs %0, DAIF\n" : "=r"(daif)::"memory");
	/* do not mask fiq irq for cpu interface */
	daif &= ~(0x3 << 6);
	asm volatile("msr DAIF, %0" : : "r"(daif) : "cc");

	pr_debug("\n cpu[%u] gicr_base:0x%llx\n", cpuid, (u64)gicr_base);
	gic_wakeup_redist(true, gicr_base);

	/* Configure SGIs/PPIs as non-secure Group-1 */
	writel_relaxed(~0, rd_sgi_base + GICR_IGROUPR0);
	writel_relaxed(0, rd_sgi_base + GICR_IGRPMODR0);
	write_sysreg_s(ICC_IGRPEN0_EL1_MASK, SYS_ICC_IGRPEN0_EL1);
	write_sysreg_s(ICC_IGRPEN1_EL1_MASK, SYS_ICC_IGRPEN1_EL1);
	// write_sysreg_s((0x1 << 0), SYS_ICC_IGRPEN1_EL3);

	write_sysreg_s(0xff, SYS_ICC_PMR_EL1);

	isb();

	gic_cpu_config(rd_sgi_base, gicr_wait_for_rwp);

	/* initialise system registers */
	gic_cpu_sys_reg_init();
}

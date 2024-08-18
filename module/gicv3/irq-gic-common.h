#ifndef __LINUX_IRQCHIP_ARM_GIC_COMMON_H
#define __LINUX_IRQCHIP_ARM_GIC_COMMON_H

#define GICD_INT_DEF_PRI		0xa0
#define GICD_INT_DEF_PRI_X4		((GICD_INT_DEF_PRI << 24) |\
					(GICD_INT_DEF_PRI << 16) |\
					(GICD_INT_DEF_PRI << 8) |\
					GICD_INT_DEF_PRI)

enum gic_type {
	GIC_V2,
	GIC_V3,
};

extern void gic_dist_config(void *base, int gic_irqs, void (*sync_access)(void));
extern void gic_cpu_config(void *base, void (*sync_access)(void));
extern int gic_configure_irq(unsigned int irq, unsigned int type, void *base,
			     void (*sync_access)(void));
#endif /* __LINUX_IRQCHIP_ARM_GIC_COMMON_H */

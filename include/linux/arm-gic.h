/*
 *  include/linux/irqchip/arm-gic.h
 *
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LINUX_IRQCHIP_ARM_GIC_H
#define __LINUX_IRQCHIP_ARM_GIC_H

#define GIC_CPU_CTRL			0x00
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18
#define GIC_CPU_ALIAS_BINPOINT		0x1c
#define GIC_CPU_ACTIVEPRIO		0xd0
#define GIC_CPU_IDENT			0xfc
#define GIC_CPU_DEACTIVATE		0x1000

#define GICC_ENABLE			0x1
#define GICC_INT_PRI_THRESHOLD		0xf0

#define GIC_CPU_CTRL_EnableGrp0_SHIFT	0
#define GIC_CPU_CTRL_EnableGrp0		(1 << GIC_CPU_CTRL_EnableGrp0_SHIFT)
#define GIC_CPU_CTRL_EnableGrp1_SHIFT	1
#define GIC_CPU_CTRL_EnableGrp1		(1 << GIC_CPU_CTRL_EnableGrp1_SHIFT)
#define GIC_CPU_CTRL_AckCtl_SHIFT	2
#define GIC_CPU_CTRL_AckCtl		(1 << GIC_CPU_CTRL_AckCtl_SHIFT)
#define GIC_CPU_CTRL_FIQEn_SHIFT	3
#define GIC_CPU_CTRL_FIQEn		(1 << GIC_CPU_CTRL_FIQEn_SHIFT)
#define GIC_CPU_CTRL_CBPR_SHIFT		4
#define GIC_CPU_CTRL_CBPR		(1 << GIC_CPU_CTRL_CBPR_SHIFT)
#define GIC_CPU_CTRL_EOImodeNS_SHIFT	9
#define GIC_CPU_CTRL_EOImodeNS		(1 << GIC_CPU_CTRL_EOImodeNS_SHIFT)

#define GICC_IAR_INT_ID_MASK		0x3ff
#define GICC_INT_SPURIOUS		1023
#define GICC_DIS_BYPASS_MASK		0x1e0

#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_IIDR			0x008
#define GIC_DIST_IGROUP			0x080
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_SET		0x300
#define GIC_DIST_ACTIVE_CLEAR		0x380
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00
#define GIC_DIST_SGI_PENDING_CLEAR	0xf10
#define GIC_DIST_SGI_PENDING_SET	0xf20

#define GICD_ENABLE			0x1
#define GICD_DISABLE			0x0
#define GICD_INT_ACTLOW_LVLTRIG		0x0
#define GICD_INT_EN_CLR_X32		0xffffffff
#define GICD_INT_EN_SET_SGI		0x0000ffff
#define GICD_INT_EN_CLR_PPI		0xffff0000

#define GICD_IIDR_IMPLEMENTER_SHIFT	0
#define GICD_IIDR_IMPLEMENTER_MASK	(0xfff << GICD_IIDR_IMPLEMENTER_SHIFT)
#define GICD_IIDR_REVISION_SHIFT	12
#define GICD_IIDR_REVISION_MASK		(0xf << GICD_IIDR_REVISION_SHIFT)
#define GICD_IIDR_VARIANT_SHIFT		16
#define GICD_IIDR_VARIANT_MASK		(0xf << GICD_IIDR_VARIANT_SHIFT)
#define GICD_IIDR_PRODUCT_ID_SHIFT	24
#define GICD_IIDR_PRODUCT_ID_MASK	(0xff << GICD_IIDR_PRODUCT_ID_SHIFT)


#define GICH_HCR			0x0
#define GICH_VTR			0x4
#define GICH_VMCR			0x8
#define GICH_MISR			0x10
#define GICH_EISR0 			0x20
#define GICH_EISR1 			0x24
#define GICH_ELRSR0 			0x30
#define GICH_ELRSR1 			0x34
#define GICH_APR			0xf0
#define GICH_LR0			0x100

#define GICH_HCR_EN			(1 << 0)
#define GICH_HCR_UIE			(1 << 1)
#define GICH_HCR_NPIE			(1 << 3)

#define GICH_LR_VIRTUALID		(0x3ff << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(0x3ff << GICH_LR_PHYSID_CPUID_SHIFT)
#define GICH_LR_PRIORITY_SHIFT		23
#define GICH_LR_STATE			(3 << 28)
#define GICH_LR_PENDING_BIT		(1 << 28)
#define GICH_LR_ACTIVE_BIT		(1 << 29)
#define GICH_LR_EOI			(1 << 19)
#define GICH_LR_GROUP1			(1 << 30)
#define GICH_LR_HW			(1 << 31)

#define GICH_VMCR_ENABLE_GRP0_SHIFT	0
#define GICH_VMCR_ENABLE_GRP0_MASK	(1 << GICH_VMCR_ENABLE_GRP0_SHIFT)
#define GICH_VMCR_ENABLE_GRP1_SHIFT	1
#define GICH_VMCR_ENABLE_GRP1_MASK	(1 << GICH_VMCR_ENABLE_GRP1_SHIFT)
#define GICH_VMCR_ACK_CTL_SHIFT		2
#define GICH_VMCR_ACK_CTL_MASK		(1 << GICH_VMCR_ACK_CTL_SHIFT)
#define GICH_VMCR_FIQ_EN_SHIFT		3
#define GICH_VMCR_FIQ_EN_MASK		(1 << GICH_VMCR_FIQ_EN_SHIFT)
#define GICH_VMCR_CBPR_SHIFT		4
#define GICH_VMCR_CBPR_MASK		(1 << GICH_VMCR_CBPR_SHIFT)
#define GICH_VMCR_EOI_MODE_SHIFT	9
#define GICH_VMCR_EOI_MODE_MASK		(1 << GICH_VMCR_EOI_MODE_SHIFT)

#define GICH_VMCR_PRIMASK_SHIFT		27
#define GICH_VMCR_PRIMASK_MASK		(0x1f << GICH_VMCR_PRIMASK_SHIFT)
#define GICH_VMCR_BINPOINT_SHIFT	21
#define GICH_VMCR_BINPOINT_MASK		(0x7 << GICH_VMCR_BINPOINT_SHIFT)
#define GICH_VMCR_ALIAS_BINPOINT_SHIFT	18
#define GICH_VMCR_ALIAS_BINPOINT_MASK	(0x7 << GICH_VMCR_ALIAS_BINPOINT_SHIFT)

#define GICH_MISR_EOI			(1 << 0)
#define GICH_MISR_U			(1 << 1)

#define GICV_PMR_PRIORITY_SHIFT		3
#define GICV_PMR_PRIORITY_MASK		(0x1f << GICV_PMR_PRIORITY_SHIFT)

#endif

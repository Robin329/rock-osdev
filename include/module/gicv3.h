#ifndef __GICV3_H__
#define __GICV3_H__

// Interrupt Configruation Registers
#define CPU_GIC_LEVEL_SENSITIVE (0 << 0)
#define CPU_GIC_EDGE_TRIGGERED (1 << 0)
#define CPU_GIC_N_N_MODEL (0 << 1)
#define CPU_GIC_1_N_MODEL (1 << 1)

int module_gicv3_test(int case_id, unsigned long *arg_list, int argc);
void module_gicv3_test_help(void);

#endif /* __GICV3_H__ */
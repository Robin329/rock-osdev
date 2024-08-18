#ifndef __ASM_ARM_SYSTEM_H__
#define __ASM_ARM_SYSTEM_H__
#define wfi() ({ asm volatile("wfi" : : : "memory"); })

static inline unsigned int current_el(void) {
  unsigned long el;

  asm volatile("mrs %0, CurrentEL" : "=r"(el) : : "cc");
  return 3 & (el >> 2);
}

static inline unsigned int get_sctlr(void) {
  unsigned int el;
  unsigned long val;

  el = current_el();
  if (el == 1)
    asm volatile("mrs %0, sctlr_el1" : "=r"(val) : : "cc");
  else if (el == 2)
    asm volatile("mrs %0, sctlr_el2" : "=r"(val) : : "cc");
  else
    asm volatile("mrs %0, sctlr_el3" : "=r"(val) : : "cc");

  return val;
}

static inline void set_sctlr(unsigned long val) {
  unsigned int el;

  el = current_el();
  if (el == 1)
    asm volatile("msr sctlr_el1, %0" : : "r"(val) : "cc");
  else if (el == 2)
    asm volatile("msr sctlr_el2, %0" : : "r"(val) : "cc");
  else
    asm volatile("msr sctlr_el3, %0" : : "r"(val) : "cc");

  asm volatile("isb");
}

static inline unsigned long read_mpidr(void) {
  unsigned long val;

  asm volatile("mrs %0, mpidr_el1" : "=r"(val));

  return val;
}
#endif /* __ASM_ARM_SYSTEM_H__ */
/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */
#include "uart.h"
#include "system.h"

void printf(char *, ...);
void show_reg(char *, int *);

int limit = 50;

void talker(void) {
  if (limit < 0)
    return;
  if (!limit) {
    for (;;)
      printf("%s:%d hello \n", __func__, __LINE__);
  } else {
    for (; limit--;)
      printf("%s:%d hello \n", __func__, __LINE__);
  }
}

void show_stack(int recursion) {
  int dummy;

  show_reg("Stack: ", &dummy);
  if (recursion)
    show_stack(0);
}

int bss_test[128];

void verify_bss(void) {
  int i;
  int bss_bad = 0;

  for (i = 0; i < 128; i++) {
    if (bss_test[i]) {
      printf("BSS %d: %h\n", i, bss_test[i]);
      bss_bad = 1;
    }
  }
  if (!bss_bad)
    printf("BSS ok\n");
}

void main(void) {
  uart_init();

  /* This will run the hello demo */
  talker ();

  /* This will check the stack address */
  show_stack(1);
  verify_bss();
  printf("current EL:%d\n", current_el());
  printf("     MPIDR:0x%08x\n", read_mpidr());
  /* NOTREACHED */
}

/* THE END */

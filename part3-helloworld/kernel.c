/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */
#include "uart.h"

int limit = 10;

void main(void) {
  uart_init();

  if (!limit) {
    for (;;)
      uart_puts("hello 1\n");
  } else {
    for (; limit--;) {
      uart_puts("hello 2\n");
      if (!limit)
        break;
    }
  }
  while (1)
    ;
  /* NOTREACHED */
}

/* THE END */

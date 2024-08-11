/* Simple serial driver for the Rockchip RK3399
 *
 * Tom Trebisky  1-2-2022
 */

#include "uart.h"

void uart_putc(char c) {
  struct rock_uart *up = UART_BASE;

  while (!(up->status & ST_TNF))
    ;

  up->data = c;
}

/* Portable code below here */

void uart_puts(char *s) {
  while (*s) {
    if (*s == '\n')
      uart_putc('\r');
    uart_putc(*s++);
  }
}

void uart_init(void) { /* nothing here, we rely on U-Boot */ }
/* THE END */

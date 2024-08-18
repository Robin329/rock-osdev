/* Simple serial driver for the Rockchip RK3399
 *
 * Tom Trebisky  1-2-2022
 */

#include "uart.h"

void putc(const char c)
{
	struct rock_uart *up = UART_BASE;

	/* If \n, also do \r */
	if (c == '\n')
		putc('\r');

	while (!(readl(&up->lsr) & LSR_TEMT))
		;

	writel(c, &up->thr);
}

int getc(void)
{
	struct rock_uart *up = UART_BASE;
	while (!(readl(&up->lsr) & LSR_DR))
		;

	return readl(&up->rbr) & 0xff;
}
/* Portable code below here */

void puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			putc('\r');
		putc(*s++);
	}
}

void uart_init(void)
{ /* nothing here, we rely on U-Boot */
}
/* THE END */

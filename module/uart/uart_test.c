#include <common.h>
#include "uart.h"
#include <module/uart.h>

static int uart_register_dump_test(struct rock_uart *up)
{
	printf("            CPR:%#x\n", up->cpr);
	printf("            ver:%#x\n", up->ucv);
	printf("  peripheral_id:%#x\n", up->ctr);
	return 0;
}

int
	module_uart_test(int case_id, unsigned long *arg_list, int argc)
{
	if (argc == 0)
		return 1;

	switch (case_id) {
	case 0:
		return uart_register_dump_test((struct rock_uart *)UART_BASE);
	default:
		break;
	}

	return 0;
}


void module_uart_test_help(void)
{
	printf("uart test 0	 - printf & dump reg test\n");
}

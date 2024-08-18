#include <common.h>
#include <command.h>
#include <cli.h>
#include <module/uart.h>

static int do_uart_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret, i, case_id;
	unsigned long *arg_list;

	if (argc < 2) {
		printf("uart test help:\n\n");
		module_uart_test_help();

		return CMD_RET_SUCCESS;
	}

	pr_debug("%s: argc %d\n", __func__, argc);

	arg_list = (unsigned long *)malloc(sizeof(unsigned long) * (argc - 2));
	if (!arg_list)
		return -ENOMEM;

	for (i = 0; i < argc - 2; i++)
		arg_list[i] = simple_strtoul(argv[i + 2], NULL, 16);

	case_id = simple_strtoul(argv[1], NULL, 16);
	ret = module_uart_test(case_id, arg_list, argc - 2);
	if (ret == CMD_RET_USAGE) {
		printf("uart test help:\n\n");
		module_uart_test_help();

		ret = CMD_RET_SUCCESS;
	}

	free(arg_list);

	return ret;
}

static cmd_tbl_t cmd_uart_sub[] = {
	ROCK_CMD_MKENT(test, 6, 0, do_uart_test, "", ""),
};

static int do_uart(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_uart_sub[0], ARRAY_SIZE(cmd_uart_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

ROCK_CMD(uart, CONFIG_SYS_MAXARGS, 0, do_uart, "uart module test",
	"uart test case_id [arg0 ... arg3] - uart test case [0 - x]");

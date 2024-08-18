#include <common.h>
#include <command.h>
#include <cli.h>
#include <module/gicv3.h>

static int do_gicv3_test(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret, i, case_id;
	unsigned long *arg_list;

	if (argc < 2) {
		printf("gicv3 test help:\n\n");
		module_gicv3_test_help();

		return CMD_RET_SUCCESS;
	}

    pr_debug("%s: argc %d\n", __func__, argc);

    arg_list = (unsigned long *)malloc(sizeof(unsigned long) * (argc - 2));
    if (!arg_list)
        return -ENOMEM;

    for (i = 0; i < argc - 2; i++)
        arg_list[i] = simple_strtoul(argv[i + 2], NULL, 16);

    case_id = simple_strtoul(argv[1], NULL, 16);
	ret = module_gicv3_test(case_id, arg_list, argc - 2);
    if (ret == CMD_RET_USAGE) {
        printf("gicv3 test help:\n\n");
		module_gicv3_test_help();

        ret = CMD_RET_SUCCESS;
    }

    free(arg_list);

    return ret;
}

static cmd_tbl_t cmd_gicv3_sub[] = {
	ROCK_CMD_MKENT(test, 6, 0, do_gicv3_test, "", ""),
};

static int do_gicv3(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if(argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_gicv3_sub[0], ARRAY_SIZE(cmd_gicv3_sub));

	if(c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

ROCK_CMD(gicv3, CONFIG_SYS_MAXARGS, 0, do_gicv3,
	   "gicv3 module test",
	   "gicv3 test case_id [arg0 ... arg3] - gicv3 test case [0 - x]");

#include <common.h>
#include <command.h>
#include <cli.h>
#include <module/display.h>

static int do_display_test(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

    return ret;
}

static cmd_tbl_t cmd_display_sub[] = {
	ROCK_CMD_MKENT(test, 6, 0, do_display_test, "", ""),
};


static int do_display(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if(argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_display_sub[0], ARRAY_SIZE(cmd_display_sub));

	if(c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

ROCK_CMD(display, CONFIG_SYS_MAXARGS, 0, do_display,
	   "XXX",
	   "XXX");
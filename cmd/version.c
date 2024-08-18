// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <linux/compiler.h>
#include <version.h>



const char __weak version_string[] = ROCK_VERSION_STRING;

static int do_version(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printf("\n%s\n", version_string);
#ifdef CC_VERSION_STRING
	puts(CC_VERSION_STRING "\n\r");
#endif
#ifdef LD_VERSION_STRING
	puts(LD_VERSION_STRING "\n\r");
#endif
	putc('\n');
	putc('\r');

	return 0;
}

ROCK_CMD(version, 1, 1, do_version,
	   "print monitor, compiler and linker version", "");

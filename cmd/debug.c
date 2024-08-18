// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <errno.h>
#include <linux/compiler.h>

extern u8 message_logleve;

static int do_debug(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    u8 level;

    if (argc == 1) {
        printf("current message loglevel: %d\n", message_logleve);
    } else {
        level = simple_strtoul(argv[1], NULL, 16);
        if (level >= 8)
            return CMD_RET_FAILURE;

        message_logleve = level;
    }

    return CMD_RET_SUCCESS;
}

ROCK_CMD(
    debug, 2, 0, do_debug,
    "debug message loglevel",
    "debug [<loglevel> 0 - 7] - show or set message loglevel"
);

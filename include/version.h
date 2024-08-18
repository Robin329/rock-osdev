/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__VERSION_H__
#define	__VERSION_H__

#include <generated/version.h>
#include <generated/utsrelease.h>
#include <generated/timestamp.h>

#define ROCK_VERSION_STRING \
"                    __             \n"\
"   _________  _____/ /______  _____\n"\
"  / ___/ __ \\/ ___/ //_/ __ \\/ ___/\n"\
" / /  / /_/ / /__/ ,< / /_/ (__  ) \n"\
"/_/   \\____/\\___/_/|_|\\____/____/  \n"\
"                                   \n"\
 PROJECT_NAME " " UTS_RELEASE " (" ROCK_DATE " - " \
	ROCK_TIME " " ROCK_TZ ")" " " CONFIG_IDENT_STRING

#ifndef __ASSEMBLY__
extern const char version_string[];
#endif	/* __ASSEMBLY__ */
#endif	/* __VERSION_H__ */

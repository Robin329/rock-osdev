/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __INITCALL_H
#define __INITCALL_H
#include <stdio.h>
typedef int (*init_fnc_t)(void);

DECLARE_GLOBAL_DATA_PTR;

/*
 * To enable debugging. add #define DEBUG at the top of the including file.
 *
 * To find a symbol, use grep on u-boot.map
 */
static inline int initcall_run_list(const init_fnc_t init_sequence[])
{
	const init_fnc_t *init_fnc_ptr;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		unsigned long reloc_ofs = 0;
		int ret;

		/*
		 * Sandbox is relocated by the OS, so symbols always appear at
		 * the relocated address.
		 */
		if (gd->flags & GD_FLG_RELOC)
			reloc_ofs = gd->reloc_off;

		if (reloc_ofs)
			pr_debug("initcall: %p (relocated to %p)\n",
					(char *)*init_fnc_ptr - reloc_ofs,
					(char *)*init_fnc_ptr);
		else
			pr_debug("initcall: %p\n", (char *)*init_fnc_ptr - reloc_ofs);

		ret = (*init_fnc_ptr)();
		if (ret) {
			printf("initcall sequence %p failed at call %p (err=%d)\n",
			       init_sequence,
			       (char *)*init_fnc_ptr - reloc_ofs, ret);
			return -1;
		}
	}
	return 0;
}

#endif

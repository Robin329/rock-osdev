// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

/**
 * These two symbols are declared in a C file so that the linker
 * uses R_ARM_RELATIVE relocation, rather than the R_ARM_ABS32 one
 * it would use if the symbols were defined in the linker file.
 * Using only R_ARM_RELATIVE relocation ensures that references to
 * the symbols are correct after as well as before relocation.
 *
 * We need a 0-byte-size type for these symbols, and the compiler
 * does not allow defining objects of C type 'void'. Using an empty
 * struct is allowed by the compiler, but causes gcc versions 4.4 and
 * below to complain about aliasing. Therefore we use the next best
 * thing: zero-sized arrays, which are both 0-byte-size and exempt from
 * aliasing warnings.
 */

char __bss_start[0] __attribute__((section(".__bss_start")));
char __bss_end[0] __attribute__((section(".__bss_end")));
char __image_copy_start[0] __attribute__((section(".__image_copy_start")));
char __image_copy_end[0] __attribute__((section(".__image_copy_end")));
char __rel_dyn_start[0] __attribute__((section(".__rel_dyn_start")));
char __rel_dyn_end[0] __attribute__((section(".__rel_dyn_end")));
char _end[0] __attribute__((section(".__end")));

.global switch_to_el1_g
.global switch_to_el2_g
.global switch_to_el0_g

.global hw_printf
.macro armv8_switch_to_el2_m_g, xreg1
	/* 64bit EL2 | HCE | SMD | RES1 (Bits[5:4]) | Non-secure EL0/EL1 */
	mov	\xreg1, #0x5b1
	msr	scr_el3, \xreg1
	msr	cptr_el3, xzr	/* Disable coprocessor traps to EL3 */
	mov	\xreg1, #0x33ff
	msr	cptr_el2, \xreg1	/* Disable coprocessor traps to EL2 */

	/* Initialize Generic Timers */
	msr	cntvoff_el2, xzr

	/* Initialize SCTLR_EL2
	 *
	 * setting RES1 bits (29,28,23,22,18,16,11,5,4) to 1
	 * and RES0 bits (31,30,27,26,24,21,20,17,15-13,10-6) +
	 * EE,WXN,I,SA,C,A,M to 0
	 */
	mov	 \xreg1, #0x0830
	movk	\xreg1, #0x30C5, lsl #16
	msr	sctlr_el2, \xreg1

	/* Return to the EL2_SP2 mode from EL3 */
	mov	\xreg1, sp
	msr	sp_el2, \xreg1		/* Migrate SP */
	mrs	\xreg1, vbar_el3
	msr	vbar_el2, \xreg1		/* Migrate VBAR */
	mov	\xreg1, #0x3c9
	msr	spsr_el3, \xreg1		/* EL2_SP2 | D | A | I | F */
	msr	elr_el3, lr
	eret
.endm

.macro armv8_switch_to_el1_m_g, xreg1
	/* Initialize Generic Timers */
	mrs	\xreg1, cnthctl_el2
	orr	\xreg1, \xreg1, #0x3	/* Enable EL1 access to timers */
	msr	cnthctl_el2, \xreg1
	msr	cntvoff_el2, xzr

	/* Initilize MPID/MPIDR registers */
	mrs	\xreg1, midr_el1
	msr	vpidr_el2, \xreg1
	mrs	\xreg1, mpidr_el1
	msr	vmpidr_el2, \xreg1

	/* Disable coprocessor traps */
	mov	\xreg1, #0x33ff
	msr	cptr_el2, \xreg1	/* Disable coprocessor traps to EL2 */
	msr	hstr_el2, xzr		/* Disable coprocessor traps to EL2 */
	mov	\xreg1, #3 << 20
	msr	cpacr_el1, \xreg1	/* Enable FP/SIMD at EL1 */

	/* Initialize HCR_EL2 */
	mov	\xreg1, #(1 << 31)		/* 64bit EL1 */
	orr	\xreg1, \xreg1, #(1 << 29)	/* Disable HVC */
    orr \xreg1, \xreg1, #(1 << 3) //FMO
    orr \xreg1, \xreg1, #(1 << 4) //FMO
	msr	hcr_el2, \xreg1

	/* SCTLR_EL1 initialization
	 *
	 * setting RES1 bits (29,28,23,22,20,11) to 1
	 * and RES0 bits (31,30,27,21,17,13,10,6) +
	 * UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
	 * CP15BEN,SA0,SA,C,A,M to 0
	 */
	mov	\xreg1, #0x0800
	movk	\xreg1, #0x30d0, lsl #16
	msr	sctlr_el1, \xreg1

	/* Return to the EL1_SP1 mode from EL2 */
	mov	\xreg1, sp
	msr	sp_el1, \xreg1		/* Migrate SP */
	mrs	\xreg1, vbar_el2
	msr	vbar_el1, \xreg1	/* Migrate VBAR */
	mov	\xreg1, #0x3c5
	msr	spsr_el2, \xreg1	/* EL1_SP1 | D | A | I | F */
	msr	elr_el2, lr
	eret
.endm

.macro armv8_switch_to_el0_m_g, xreg1
	/* Return to the EL1_SP1 mode from EL2 */
	mov	\xreg1, sp
	msr	sp_el0, \xreg1		/* Migrate SP */
	mrs	\xreg1, spsr_el1
	bic	\xreg1,\xreg1, #0xf
	msr	spsr_el1, \xreg1	/* EL1_SP0 | D | A  */
	msr	elr_el1, lr
	eret
.endm

.type switch_to_el2_g, %function
switch_to_el2_g:
    armv8_switch_to_el2_m_g x0

.type switch_to_el1_g, %function
switch_to_el1_g:
    armv8_switch_to_el1_m_g x0

.type switch_to_el0_g, %function
switch_to_el0_g:
    armv8_switch_to_el0_m_g x0


#include <common.h>

typedef volatile unsigned int vu32;
typedef unsigned int u32;

/* The TRM describes the UART in chapter 19
 */
struct rock_uart {
	union {
		uint32_t	rbr; /* 0x0 */
		uint32_t	dll;
		uint32_t	thr;
	};
	union {
		uint32_t	dlh; /* 0x4 */
		uint32_t	ier;
	};
	union {
		uint32_t	fcr; /* 0x8 */
		uint32_t	iir;
	};
	uint32_t	lcr; /* 0xc */
	uint32_t	mcr; /* 0x10 */
	uint32_t	lsr; /* 0x14 */
	uint32_t	msr; /* 0x18 */
	uint32_t	scr; /* 0x1c */
	uint32_t	lpdll; /* 0x20 */
  uint8_t	reserved0[0x30 - 0x24];
  vu32 srbr; /* 0x30 */
  u32 _pad1[3];

  u32 _pad2[12];

  vu32 far;    /* 0x70 */
  vu32 tfr;    /* 0x74 */
  vu32 rfw;    /* 0x78 */
  vu32 status; /* 0x7c */

  vu32 tfl;  /* 0x80 */
  vu32 rfl;  /* 0x84 */
  vu32 srr;  /* 0x88 */
  vu32 srts; /* 0x8c */

  vu32 sbcr;  /* 0x90 */
  vu32 sdmam; /* 0x94 */
  vu32 sfe;   /* 0x98 */
  vu32 srt;   /* 0x9c */

  vu32 stet;  /* 0xa0 */
  vu32 htx;   /* 0xa4 */
  vu32 dmasa; /* 0xa8 */
  u32 _pad3;  /* 0xac */

  u32 _pad4[16];

  u32 _pad5; /* 0xf0 */
  vu32 cpr;  /* 0xf4 */
  vu32 ucv;  /* 0xf8 */
  vu32 ctr;  /* 0xfc */
};

#define UART0_BASE ((struct rock_uart *)0xFD890000)
#define UART1_BASE ((struct rock_uart *)0xFEB40000)
#define UART2_BASE ((struct rock_uart *)0xFEB50000)
#define UART3_BASE ((struct rock_uart *)0xFEB60000)
#define UART4_BASE ((struct rock_uart *)0xFEB70000)
#define UART5_BASE ((struct rock_uart *)0xFEB80000)
#define UART6_BASE ((struct rock_uart *)0xFEB90000)
#define UART7_BASE ((struct rock_uart *)0xFEBA0000)
#define UART8_BASE ((struct rock_uart *)0xFEBB0000)
#define UART9_BASE ((struct rock_uart *)0xFEBC0000)

#define UART_BASE UART2_BASE

#define	IER_DMAE	BIT(7)
#define	IER_UUE		BIT(6)
#define	IER_NRZE	BIT(5)
#define	IER_RTIOE	BIT(4)
#define	IER_MIE		BIT(3)
#define	IER_RLSE	BIT(2)
#define	IER_TIE		BIT(1)
#define	IER_RAVIE	BIT(0)

#define	IIR_FIFOES1	BIT(7)
#define	IIR_FIFOES0	BIT(6)
#define	IIR_TOD		BIT(3)
#define	IIR_IID2	BIT(2)
#define	IIR_IID1	BIT(1)
#define	IIR_IP		BIT(0)

#define	FCR_ITL2	BIT(7)
#define	FCR_ITL1	BIT(6)
#define	FCR_RESETTF	BIT(2)
#define	FCR_RESETRF	BIT(1)
#define	FCR_TRFIFOE	BIT(0)
#define	FCR_ITL_1	0
#define	FCR_ITL_8	(FCR_ITL1)
#define	FCR_ITL_16	(FCR_ITL2)
#define	FCR_ITL_32	(FCR_ITL2|FCR_ITL1)

#define	LCR_DLAB	BIT(7)
#define	LCR_SB		BIT(6)
#define	LCR_STKYP	BIT(5)
#define	LCR_EPS		BIT(4)
#define	LCR_PEN		BIT(3)
#define	LCR_STB		BIT(2)
#define	LCR_WLS1	BIT(1)
#define	LCR_WLS0	BIT(0)

#define	LSR_FIFOE	BIT(7)
#define	LSR_TEMT	BIT(6)
#define	LSR_TDRQ	BIT(5)
#define	LSR_BI		BIT(4)
#define	LSR_FE		BIT(3)
#define	LSR_PE		BIT(2)
#define	LSR_OE		BIT(1)
#define	LSR_DR		BIT(0)

#define	MCR_LOOP	BIT(4)
#define	MCR_OUT2	BIT(3)
#define	MCR_OUT1	BIT(2)
#define	MCR_RTS		BIT(1)
#define	MCR_DTR		BIT(0)

#define	MSR_DCD		BIT(7)
#define	MSR_RI		BIT(6)
#define	MSR_DSR		BIT(5)
#define	MSR_CTS		BIT(4)
#define	MSR_DDCD	BIT(3)
#define	MSR_TERI	BIT(2)
#define	MSR_DDSR	BIT(1)
#define	MSR_DCTS	BIT(0)

#define USR_RFF		BIT(4)
#define USR_RFNE	BIT(3)
#define USR_TFE		BIT(2)
#define USR_TFNF	BIT(1)
#define USR_BUSY	BIT(0)

void uart_init(void);
void putc(const char c);
void puts(const char *);
void show_reg(char *, int *);
int getc(void);
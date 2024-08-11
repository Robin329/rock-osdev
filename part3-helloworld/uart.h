typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x) (1 << (x))

/* The TRM describes the UART in chapter 19
 */
struct rock_uart {
  vu32 data;
  vu32 ier;
  vu32 iir;
  vu32 lcr;

  vu32 mcr;
  vu32 lsr;
  vu32 msr;
  vu32 scr;

  u32 _pad0[4];

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

/* Bits in the status register */
#define ST_BUSY      BIT(0)
#define ST_TNF       BIT(1)
#define ST_TE        BIT(2)
#define ST_RNE       BIT(3)
#define ST_RF        BIT(4)

void uart_init(void);
void uart_putc(char c);
void uart_puts(char *);
void show_reg(char *, int *);
/* serial.c
 * (c) Tom Trebisky  7-2-2017
 *
 * Serial (uart) driver for the F411
 * For the 411 this is section 19 of RM0383
 *
 * This began (2017) as a simple polled output driver for
 *  console messages on port 1
 * In 2020, I decided to extend it to listen to a GPS receiver
 *  on port 2.
 *
 * Notice that I number these 1,2,3.
 * However my "3" is what they call "6" in the manual.
 *
 * On the F411, USART1 and USART6 are on the APB2 bus.
 * On the F411, USART2 is on the APB1 bus.
 *
 * On the F411, after reset, with no fiddling with RCC
 *  settings, both are running at 16 Mhz.
 *  Apparently on the F411 both APB1 and APB2
 *   always run at the same rate.
 *
 * NOTE: On my black pill boards, pins C6 and C7 are not available,
 *  Meaning that UART3 (aka UART6) is not available.
 *  The code is in here, but not of much use if you can't
 *  get to the pins!!
 */

#include "uart.h"
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#if 0
#define PRINTF_BUF_SIZE 128
static void asnprintf (char *abuf, unsigned int size, const char *fmt, va_list args);

void
printf ( char *fmt, ... )
{
	char buf[PRINTF_BUF_SIZE];
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, PRINTF_BUF_SIZE, fmt, args );
        va_end ( args );

        puts ( buf );
}

/* The limit is absurd, so take care */
void
sprintf ( char *buf, char *fmt, ... )
{
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, 256, fmt, args );
        va_end ( args );
}

/* ========================================================================= */

/* Here I develop a simple printf.
 * It only has 3 triggers:
 *  %s to inject a string
 *  %d to inject a decimal number
 *  %h to inject a 32 bit hex value as xxxxyyyy
 */

#define PUTCHAR(x)      if ( buf <= end ) *buf++ = (x)

static const char hex_table[] = "0123456789ABCDEF";

// #define HEX(x)  ((x)<10 ? '0'+(x) : 'A'+(x)-10)
#define HEX(x)  hex_table[(x)]

#ifdef notdef
static char *
sprintnb ( char *buf, char *end, int n, int b)
{
        char prbuf[16];
        register char *cp;

        if (b == 10 && n < 0) {
            PUTCHAR('-');
            n = -n;
        }
        cp = prbuf;

        do {
            // *cp++ = "0123456789ABCDEF"[n%b];
            *cp++ = hex_table[n%b];
            n /= b;
        } while (n);

        do {
            PUTCHAR(*--cp);
        } while (cp > prbuf);

        return buf;
}
#endif

static char *
sprintn ( char *buf, char *end, int n )
{
        char prbuf[16];
        char *cp;

        if ( n < 0 ) {
            PUTCHAR('-');
            n = -n;
        }
        cp = prbuf;

        do {
            // *cp++ = "0123456789"[n%10];
            *cp++ = hex_table[n%10];
            n /= 10;
        } while (n);

        do {
            PUTCHAR(*--cp);
        } while (cp > prbuf);

        return buf;
}

static char *
shex2( char *buf, char *end, int val )
{
        PUTCHAR( HEX((val>>4)&0xf) );
        PUTCHAR( HEX(val&0xf) );
        return buf;
}

#ifdef notdef
static char *
shex3( char *buf, char *end, int val )
{
        PUTCHAR( HEX((val>>8)&0xf) );
        return shex2(buf,end,val);
}

static char *
shex4( char *buf, char *end, int val )
{
        buf = shex2(buf,end,val>>8);
        return shex2(buf,end,val);
}
#endif

static char *
shex8( char *buf, char *end, int val )
{
        buf = shex2(buf,end,val>>24);
        buf = shex2(buf,end,val>>16);
        buf = shex2(buf,end,val>>8);
        return shex2(buf,end,val);
}

static void
asnprintf (char *abuf, unsigned int size, const char *fmt, va_list args)
{
    char *buf, *end;
    int c;
    char *p;

    buf = abuf;
    end = buf + size - 1;
    if (end < buf - 1) {
        end = ((void *) -1);
        size = end - buf + 1;
    }

    while ( c = *fmt++ ) {
	if ( c != '%' ) {
            PUTCHAR(c);
            continue;
        }
	c = *fmt++;
	if ( c == 'd' ) {
	    buf = sprintn ( buf, end, va_arg(args,int) );
	    continue;
	}
	if ( c == 'x' ) {
	    buf = shex2 ( buf, end, va_arg(args,int) & 0xff );
	    continue;
	}
	if ( c == 'h' || c == 'X' ) {
	    buf = shex8 ( buf, end, va_arg(args,int) );
	    continue;
	}
	if ( c == 'c' ) {
            PUTCHAR( va_arg(args,int) );
	    continue;
	}
	if ( c == 's' ) {
	    p = va_arg(args,char *);
	    // printf ( "Got: %s\n", p );
	    while ( c = *p++ )
		PUTCHAR(c);
	    continue;
	}
    }
    if ( buf > end )
	buf = end;
    PUTCHAR('\0');
}

#ifdef notdef
void
serial_printf ( int fd, char *fmt, ... )
{
	char buf[PRINTF_BUF_SIZE];
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, PRINTF_BUF_SIZE, fmt, args );
        va_end ( args );

        puts ( fd, buf );
}
#endif

#else
/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */
#define NULL ((void *)0)
#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-MAX_ERRNO)
static inline long PTR_ERR(const void *ptr) {
  return ((long)ptr - 0x0);
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)PTR_ERR(ptr));
}

typedef __SIZE_TYPE__ size_t;
#define do_div(n, base)                                                        \
  ({                                                                           \
    u32 __base = (base);                                                       \
    u32 __rem;                                                                 \
    __rem = ((unsigned long long)(n)) % __base;                                \
    (n) = ((unsigned long long)(n)) / __base;                                  \
    __rem;                                                                     \
  })

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif

typedef __s8 s8;
typedef __u8 u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;

/* bsd */
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

/* sysv */
typedef unsigned char unchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef u8 u_int8_t;
typedef s8 int8_t;
typedef u16 u_int16_t;
typedef s16 int16_t;
typedef u32 u_int32_t;
typedef s32 int32_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;

#if defined(__GNUC__)
typedef u64 uint64_t;
typedef u64 u_int64_t;
typedef s64 int64_t;
#endif
#ifdef __GNUC__
typedef long long __kernel_loff_t;
#endif
#if defined(__GNUC__)
typedef __kernel_loff_t loff_t;
#endif
typedef unsigned long __kernel_size_t;
typedef long __kernel_ssize_t;
typedef long __kernel_ptrdiff_t;

#define USHRT_MAX ((u16)(~0U))
#define SHRT_MAX ((s16)(USHRT_MAX >> 1))
#define SHRT_MIN ((s16)(-SHRT_MAX - 1))
#define INT_MAX ((int)(~0U >> 1))
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX (~0U)
#define LONG_MAX ((long)(~0UL >> 1))
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX (~0UL)
#define LLONG_MAX ((long long)(~0ULL >> 1))
#define LLONG_MIN (-LLONG_MAX - 1)
#define ULLONG_MAX (~0ULL)
#ifndef SIZE_MAX
#define SIZE_MAX (~(size_t)0)
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX ((ssize_t)(SIZE_MAX >> 1))
#endif

#define U8_MAX ((u8)~0U)
#define S8_MAX ((s8)(U8_MAX >> 1))
#define S8_MIN ((s8)(-S8_MAX - 1))
#define U16_MAX ((u16)~0U)
#define S16_MAX ((s16)(U16_MAX >> 1))
#define S16_MIN ((s16)(-S16_MAX - 1))
#define U32_MAX ((u32)~0U)
#define S32_MAX ((s32)(U32_MAX >> 1))
#define S32_MIN ((s32)(-S32_MAX - 1))
#define U64_MAX ((u64)~0ULL)
#define S64_MAX ((s64)(U64_MAX >> 1))
#define S64_MIN ((s64)(-S64_MAX - 1))
#define _U 0x01  /* upper */
#define _L 0x02  /* lower */
#define _D 0x04  /* digit */
#define _C 0x08  /* cntrl */
#define _P 0x10  /* punct */
#define _S 0x20  /* white space (space/lf/tab) */
#define _X 0x40  /* hex digit */
#define _SP 0x80 /* hard space (0x20) */

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)                                          \
  ({                                                                           \
    typeof(x) __x = x;                                                         \
    typeof(divisor) __d = divisor;                                             \
    (((typeof(x))-1) > 0 || ((typeof(divisor))-1) > 0 || (__x) > 0)            \
        ? (((__x) + ((__d) / 2)) / (__d))                                      \
        : (((__x) - ((__d) / 2)) / (__d));                                     \
  })

const unsigned char
    _ctype[] = {_C,       _C,      _C,      _C,      _C,      _C,
                _C,       _C, /* 0-7 */
                _C,       _C | _S, _C | _S, _C | _S, _C | _S, _C | _S,
                _C,       _C, /* 8-15 */
                _C,       _C,      _C,      _C,      _C,      _C,
                _C,       _C, /* 16-23 */
                _C,       _C,      _C,      _C,      _C,      _C,
                _C,       _C, /* 24-31 */
                _S | _SP, _P,      _P,      _P,      _P,      _P,
                _P,       _P, /* 32-39 */
                _P,       _P,      _P,      _P,      _P,      _P,
                _P,       _P, /* 40-47 */
                _D,       _D,      _D,      _D,      _D,      _D,
                _D,       _D, /* 48-55 */
                _D,       _D,      _P,      _P,      _P,      _P,
                _P,       _P, /* 56-63 */
                _P,       _U | _X, _U | _X, _U | _X, _U | _X, _U | _X,
                _U | _X,  _U, /* 64-71 */
                _U,       _U,      _U,      _U,      _U,      _U,
                _U,       _U, /* 72-79 */
                _U,       _U,      _U,      _U,      _U,      _U,
                _U,       _U, /* 80-87 */
                _U,       _U,      _U,      _P,      _P,      _P,
                _P,       _P, /* 88-95 */
                _P,       _L | _X, _L | _X, _L | _X, _L | _X, _L | _X,
                _L | _X,  _L, /* 96-103 */
                _L,       _L,      _L,      _L,      _L,      _L,
                _L,       _L, /* 104-111 */
                _L,       _L,      _L,      _L,      _L,      _L,
                _L,       _L, /* 112-119 */
                _L,       _L,      _L,      _P,      _P,      _P,
                _P,       _C, /* 120-127 */
                0,        0,       0,       0,       0,       0,
                0,        0,       0,       0,       0,       0,
                0,        0,       0,       0, /* 128-143 */
                0,        0,       0,       0,       0,       0,
                0,        0,       0,       0,       0,       0,
                0,        0,       0,       0, /* 144-159 */
                _S | _SP, _P,      _P,      _P,      _P,      _P,
                _P,       _P,      _P,      _P,      _P,      _P,
                _P,       _P,      _P,      _P, /* 160-175 */
                _P,       _P,      _P,      _P,      _P,      _P,
                _P,       _P,      _P,      _P,      _P,      _P,
                _P,       _P,      _P,      _P, /* 176-191 */
                _U,       _U,      _U,      _U,      _U,      _U,
                _U,       _U,      _U,      _U,      _U,      _U,
                _U,       _U,      _U,      _U, /* 192-207 */
                _U,       _U,      _U,      _U,      _U,      _U,
                _U,       _P,      _U,      _U,      _U,      _U,
                _U,       _U,      _U,      _L, /* 208-223 */
                _L,       _L,      _L,      _L,      _L,      _L,
                _L,       _L,      _L,      _L,      _L,      _L,
                _L,       _L,      _L,      _L, /* 224-239 */
                _L,       _L,      _L,      _L,      _L,      _L,
                _L,       _P,      _L,      _L,      _L,      _L,
                _L,       _L,      _L,      _L}; /* 240-255 */
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c) ((__ismask(c) & (_U | _L | _D)) != 0)
#define isalpha(c) ((__ismask(c) & (_U | _L)) != 0)
#define iscntrl(c) ((__ismask(c) & (_C)) != 0)
#define isdigit(c) ((__ismask(c) & (_D)) != 0)
#define isgraph(c) ((__ismask(c) & (_P | _U | _L | _D)) != 0)
#define islower(c) ((__ismask(c) & (_L)) != 0)
#define isprint(c) ((__ismask(c) & (_P | _U | _L | _D | _SP)) != 0)
#define ispunct(c) ((__ismask(c) & (_P)) != 0)
#define isspace(c) ((__ismask(c) & (_S)) != 0)
#define isupper(c) ((__ismask(c) & (_U)) != 0)
#define isxdigit(c) ((__ismask(c) & (_D | _X)) != 0)

#define MAX_LINE_LENGTH_BYTES 64

const char hex_asc[] = "0123456789abcdef";
const char hex_asc_upper[] = "0123456789ABCDEF";

static inline unsigned char __tolower(unsigned char c) {
  if (isupper(c))
    c -= 'A' - 'a';
  return c;
}

static inline unsigned char __toupper(unsigned char c) {
  if (islower(c))
    c -= 'a' - 'A';
  return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)
#define hex_asc_lo(x) hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x) hex_asc[((x) & 0xf0) >> 4]

static inline char *hex_byte_pack(char *buf, unsigned char byte) {
  *buf++ = hex_asc_hi(byte);
  *buf++ = hex_asc_lo(byte);
  return buf;
}

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
static inline int hex_to_bin(char ch) {
  if ((ch >= '0') && (ch <= '9'))
    return ch - '0';
  ch = tolower(ch);
  if ((ch >= 'a') && (ch <= 'f'))
    return ch - 'a' + 10;
  return -1;
}

/**
 * hex2bin - convert an ascii hexadecimal string to its binary representation
 * @dst: binary result
 * @src: ascii hexadecimal string
 * @count: result length
 *
 * Return 0 on success, -1 in case of bad input.
 */
static inline int hex2bin(unsigned char *dst, const char *src, size_t count) {
  while (count--) {
    int hi = hex_to_bin(*src++);
    int lo = hex_to_bin(*src++);

    if ((hi < 0) || (lo < 0))
      return -1;

    *dst++ = (hi << 4) | lo;
  }
  return 0;
}
/* from lib/kstrtox.c */
static const char *_parse_integer_fixup_radix(const char *s, uint *basep) {
  /* Look for a 0x prefix */
  if (s[0] == '0') {
    int ch = tolower(s[1]);

    if (ch == 'x') {
      *basep = 16;
      s += 2;
    } else if (!*basep) {
      /* Only select octal if we don't have a base */
      *basep = 8;
    }
  }

  /* Use decimal by default */
  if (!*basep)
    *basep = 10;

  return s;
}
/**
 * decode_digit() - Decode a single character into its numeric digit value
 *
 * This ignore case
 *
 * @ch: Character to convert (expects '0'..'9', 'a'..'f' or 'A'..'F')
 * Return: value of digit (0..0xf) or 255 if the character is invalid
 */
static uint decode_digit(int ch) {
  if (!isxdigit(ch))
    return 256;

  ch = tolower(ch);

  return ch <= '9' ? ch - '0' : ch - 'a' + 0xa;
}

ulong simple_strtoul(const char *cp, char **endp, uint base) {
  ulong result = 0;
  uint value;

  cp = _parse_integer_fixup_radix(cp, &base);

  while (value = decode_digit(*cp), value < base) {
    result = result * base + value;
    cp++;
  }

  if (endp)
    *endp = (char *)cp;

  return result;
}

ulong hextoul(const char *cp, char **endp) {
  return simple_strtoul(cp, endp, 16);
}
/**
 * bin2hex - convert binary data to an ascii hexadecimal string
 * @dst: ascii hexadecimal result
 * @src: binary data
 * @count: binary data length
 */
static inline char *bin2hex(char *dst, const void *src, size_t count) {
  const unsigned char *_src = src;

  while (count--)
    dst = hex_byte_pack(dst, *_src++);
  return dst;
}
/* we use this so that we can do without the ctype library */
#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s) {
  int i = 0;

  while (is_digit(**s))
    i = i * 10 + *((*s)++) - '0';

  return i;
}
size_t strnlen(const char *s, size_t maxlen) {
  size_t i;

  for (i = 0; i < maxlen; ++i)
    if (s[i] == '\0')
      break;
  return i;
}
/**
 * simple_strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
/* Works only for digits and letters, but small and fast */
#define TOLOWER(x) ((x) | 0x20)

static unsigned int simple_guess_base(const char *cp)
{
	if (cp[0] == '0') {
		if (TOLOWER(cp[1]) == 'x' && isxdigit(cp[2]))
			return 16;
		else
			return 8;
	} else {
		return 10;
	}
}

unsigned long long simple_strtoull(const char *cp, char **endp,
                                   unsigned int base) {
  unsigned long long result = 0;

  if (!base)
    base = simple_guess_base(cp);

  if (base == 16 && cp[0] == '0' && TOLOWER(cp[1]) == 'x')
    cp += 2;

  while (isxdigit(*cp)) {
    unsigned int value;

    value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
    if (value >= base)
      break;
    result = result * base + value;
    cp++;
  }
  if (endp)
    *endp = (char *)cp;

  return result;
}
/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static char *put_dec_trunc(char *buf, unsigned q) {
  unsigned d3, d2, d1, d0;
  d1 = (q >> 4) & 0xf;
  d2 = (q >> 8) & 0xf;
  d3 = (q >> 12);

  d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
  q = (d0 * 0xcd) >> 11;
  d0 = d0 - 10 * q;
  *buf++ = d0 + '0'; /* least significant digit */
  d1 = q + 9 * d3 + 5 * d2 + d1;
  if (d1 != 0) {
    q = (d1 * 0xcd) >> 11;
    d1 = d1 - 10 * q;
    *buf++ = d1 + '0'; /* next digit */

    d2 = q + 2 * d2;
    if ((d2 != 0) || (d3 != 0)) {
      q = (d2 * 0xd) >> 7;
      d2 = d2 - 10 * q;
      *buf++ = d2 + '0'; /* next digit */

      d3 = q + 4 * d3;
      if (d3 != 0) {
        q = (d3 * 0xcd) >> 11;
        d3 = d3 - 10 * q;
        *buf++ = d3 + '0'; /* next digit */
        if (q != 0)
          *buf++ = q + '0'; /* most sign. digit */
      }
    }
  }
  return buf;
}
/* Same with if's removed. Always emits five digits */
static char *put_dec_full(char *buf, unsigned q) {
  /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
  /* but anyway, gcc produces better code with full-sized ints */
  unsigned d3, d2, d1, d0;
  d1 = (q >> 4) & 0xf;
  d2 = (q >> 8) & 0xf;
  d3 = (q >> 12);

  /*
   * Possible ways to approx. divide by 10
   * gcc -O2 replaces multiply with shifts and adds
   * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
   * (x * 0x67) >> 10:  1100111
   * (x * 0x34) >> 9:    110100 - same
   * (x * 0x1a) >> 8:     11010 - same
   * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
   */

  d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
  q = (d0 * 0xcd) >> 11;
  d0 = d0 - 10 * q;
  *buf++ = d0 + '0';
  d1 = q + 9 * d3 + 5 * d2 + d1;
  q = (d1 * 0xcd) >> 11;
  d1 = d1 - 10 * q;
  *buf++ = d1 + '0';

  d2 = q + 2 * d2;
  q = (d2 * 0xd) >> 7;
  d2 = d2 - 10 * q;
  *buf++ = d2 + '0';

  d3 = q + 4 * d3;
  q = (d3 * 0xcd) >> 11; /* - shorter code */
  /* q = (d3 * 0x67) >> 10; - would also work */
  d3 = d3 - 10 * q;
  *buf++ = d3 + '0';
  *buf++ = q + '0';
  return buf;
}
/* No inlining helps gcc to use registers better */
static char *put_dec(char *buf, unsigned long long num) {
  while (1) {
    unsigned rem;
    if (num < 100000)
      return put_dec_trunc(buf, num);
    rem = do_div(num, 100000);
    buf = put_dec_full(buf, rem);
  }
}

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2     /* unsigned/signed long */
#define PLUS 4     /* show plus */
#define SPACE 8    /* space if plus */
#define LEFT 16    /* left justified */
#define SMALL 32   /* Must be 32 == 0x20 */
#define SPECIAL 64 /* 0x */
#define ERRSTR 128 /* %dE showing error string if enabled */

/*
 * Macro to add a new character to our output string, but only if it will
 * fit. The macro moves to the next character position in the output string.
 */
#define ADDCH(str, ch)                                                         \
  do {                                                                         \
    if ((str) < end)                                                           \
      *(str) = (ch);                                                           \
    ++str;                                                                     \
  } while (0)

static char *number(char *buf, char *end, unsigned long long num, int base, int size,
                    int precision, int type) {
  /* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
  static const char digits[16] = "0123456789ABCDEF";

  char tmp[66];
  char sign;
  char locase;
  int need_pfx = ((type & SPECIAL) && base != 10);
  int i;

  /* locase = 0 or 0x20. ORing digits or letters with 'locase'
   * produces same digits or (maybe lowercased) letters */
  locase = (type & SMALL);
  if (type & LEFT)
    type &= ~ZEROPAD;
  sign = 0;
  if (type & SIGN) {
    if ((long long)num < 0) {
      sign = '-';
      num = -(long long)num;
      size--;
    } else if (type & PLUS) {
      sign = '+';
      size--;
    } else if (type & SPACE) {
      sign = ' ';
      size--;
    }
  }
  if (need_pfx) {
    size--;
    if (base == 16)
      size--;
  }

  /* generate full string in tmp[], in reverse order */
  i = 0;
  if (num == 0)
    tmp[i++] = '0';
  /* Generic code, for any base:
  else do {
          tmp[i++] = (digits[do_div(num,base)] | locase);
  } while (num != 0);
  */
  else if (base != 10) { /* 8 or 16 */
    int mask = base - 1;
    int shift = 3;

    if (base == 16)
      shift = 4;

    do {
      tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
      num >>= shift;
    } while (num);
  } else { /* base 10 */
    i = put_dec(tmp, num) - tmp;
  }

  /* printing 100 using %2d gives "100", not "00" */
  if (i > precision)
    precision = i;
  /* leading space padding */
  size -= precision;
  if (!(type & (ZEROPAD + LEFT))) {
    while (--size >= 0)
      ADDCH(buf, ' ');
  }
  /* sign */
  if (sign)
    ADDCH(buf, sign);
  /* "0x" / "0" prefix */
  if (need_pfx) {
    ADDCH(buf, '0');
    if (base == 16)
      ADDCH(buf, 'X' | locase);
  }
  /* zero or space padding */
  if (!(type & LEFT)) {
    char c = (type & ZEROPAD) ? '0' : ' ';

    while (--size >= 0)
      ADDCH(buf, c);
  }
  /* hmm even more zero padding? */
  while (i <= --precision)
    ADDCH(buf, '0');
  /* actual digits of result */
  while (--i >= 0)
    ADDCH(buf, tmp[i]);
  /* trailing space padding */
  while (--size >= 0)
    ADDCH(buf, ' ');
  return buf;
}

static char *string(char *buf, char *end, const char *s, int field_width,
                    int precision, int flags) {
  int len, i;

  if (s == NULL)
    s = "<NULL>";

  len = strnlen(s, precision);

  if (!(flags & LEFT))
    while (len < field_width--)
      ADDCH(buf, ' ');
  for (i = 0; i < len; ++i)
    ADDCH(buf, *s++);
  while (len < field_width--)
    ADDCH(buf, ' ');
  return buf;
}



static char *mac_address_string(char *buf, char *end, unsigned char *addr, int field_width,
                                int precision, int flags) {
  /* (6 * 2 hex digits), 5 colons and trailing zero */
  char mac_addr[6 * 3];
  char *p = mac_addr;
  int i;

  for (i = 0; i < 6; i++) {
    p = hex_byte_pack(p, addr[i]);
    if (!(flags & SPECIAL) && i != 5)
      *p++ = ':';
  }
  *p = '\0';

  return string(buf, end, mac_addr, field_width, precision, flags & ~SPECIAL);
}

static char *ip6_addr_string(char *buf, char *end, unsigned char *addr, int field_width,
                             int precision, int flags) {
  /* (8 * 4 hex digits), 7 colons and trailing zero */
  char ip6_addr[8 * 5];
  char *p = ip6_addr;
  int i;

  for (i = 0; i < 8; i++) {
    p = hex_byte_pack(p, addr[2 * i]);
    p = hex_byte_pack(p, addr[2 * i + 1]);
    if (!(flags & SPECIAL) && i != 7)
      *p++ = ':';
  }
  *p = '\0';

  return string(buf, end, ip6_addr, field_width, precision, flags & ~SPECIAL);
}

static char *ip4_addr_string(char *buf, char *end, unsigned char *addr, int field_width,
                             int precision, int flags) {
  /* (4 * 3 decimal digits), 3 dots and trailing zero */
  char ip4_addr[4 * 4];
  char temp[3]; /* hold each IP quad in reverse order */
  char *p = ip4_addr;
  int i, digits;

  for (i = 0; i < 4; i++) {
    digits = put_dec_trunc(temp, addr[i]) - temp;
    /* reverse the digits in the quad */
    while (digits--)
      *p++ = temp[digits];
    if (i != 3)
      *p++ = '.';
  }
  *p = '\0';

  return string(buf, end, ip4_addr, field_width, precision, flags & ~SPECIAL);
}

/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way (dot-separated
 *       decimal for v4 and colon separated network-order 16 bit hex for v6)
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses, IPv6 omits the colons, IPv4 is
 *       currently the same
 */
static char *pointer(const char *fmt, char *buf, char *end, void *ptr,
                     int field_width, int precision, int flags) {
  unsigned long long num = (unsigned long int)ptr;

  /*
   * Being a boot loader, we explicitly allow pointers to
   * (physical) address null.
   */
#if 0
	if (!ptr)
		return string(buf, end, "(null)", field_width, precision,
			      flags);
#endif

  switch (*fmt) {
  case 'a':
    flags |= SPECIAL | ZEROPAD;

    switch (fmt[1]) {
    case 'p':
    default:
      field_width = sizeof(unsigned long long) * 2 + 2;
      num = *(unsigned long long *)ptr;
      break;
    }
    break;
  case 'm':
    flags |= SPECIAL;
    /* Fallthrough */
  case 'M':
    return mac_address_string(buf, end, ptr, field_width, precision, flags);
  case 'i':
    flags |= SPECIAL;
    /* Fallthrough */
  case 'I':
    if (fmt[1] == '4')
      return ip4_addr_string(buf, end, ptr, field_width, precision, flags);
    flags &= ~SPECIAL;
    break;
#ifdef CONFIG_LIB_UUID
  case 'U':
    return uuid_string(buf, end, ptr, field_width, precision, flags, fmt);
#endif
  default:
    break;
  }
  flags |= SMALL;
  if (field_width == -1) {
    field_width = 2 * sizeof(void *);
    flags |= ZEROPAD;
  }
  return number(buf, end, num, 16, field_width, precision, flags);
}

static int vsnprintf_internal(char *buf, size_t size, const char *fmt,
                              va_list args) {
  unsigned long long num;
  int base;
  char *str;

  int flags; /* flags to number() */

  int field_width; /* width of output field */
  int precision;   /* min. # of digits for integers; max
                      number of chars for from string */
  int qualifier;   /* 'h', 'l', or 'L' for integer fields */
                   /* 'z' support added 23/7/1999 S.H.    */
                   /* 'z' changed to 'Z' --davidm 1/25/99 */
                   /* 't' added for long */
  char *end = buf + size;

  /* Make sure end is always >= buf - do we want this in U-Boot? */
  if (end < buf) {
    end = ((void *)-1);
    size = end - buf;
  }
  str = buf;

  for (; *fmt; ++fmt) {
    if (*fmt != '%') {
      ADDCH(str, *fmt);
      continue;
    }

    /* process flags */
    flags = 0;
  repeat:
    ++fmt; /* this also skips first '%' */
    switch (*fmt) {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }

    /* get field width */
    field_width = -1;
    if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
    else if (*fmt == '*') {
      ++fmt;
      /* it's the next argument */
      field_width = va_arg(args, int);
      if (field_width < 0) {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.') {
      ++fmt;
      if (is_digit(*fmt))
        precision = skip_atoi(&fmt);
      else if (*fmt == '*') {
        ++fmt;
        /* it's the next argument */
        precision = va_arg(args, int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z' ||
        *fmt == 'z' || *fmt == 't') {
      qualifier = *fmt;
      ++fmt;
      if (qualifier == 'l' && *fmt == 'l') {
        qualifier = 'L';
        ++fmt;
      }
    }

    /* default base */
    base = 10;

    switch (*fmt) {
    case 'c':
      if (!(flags & LEFT)) {
        while (--field_width > 0)
          ADDCH(str, ' ');
      }
      ADDCH(str, (unsigned char)va_arg(args, int));
      while (--field_width > 0)
        ADDCH(str, ' ');
      continue;

    case 's':
      {
        str = string(str, end, va_arg(args, char *), field_width, precision,
                     flags);
      }
      continue;

    case 'p':
      str = pointer(fmt + 1, str, end, va_arg(args, void *), field_width,
                    precision, flags);
      if (IS_ERR(str))
        return PTR_ERR(str);
      /* Skip all alphanumeric pointer suffixes */
      while (isalnum(fmt[1]))
        fmt++;
      continue;

    case 'n':
      if (qualifier == 'l') {
        long *ip = va_arg(args, long *);
        *ip = (str - buf);
      } else {
        int *ip = va_arg(args, int *);
        *ip = (str - buf);
      }
      continue;

    case '%':
      ADDCH(str, '%');
      continue;

    /* integer number formats - set up the flags and "break" */
    case 'o':
      base = 8;
      break;

    case 'x':
      flags |= SMALL;
    /* fallthrough */
    case 'X':
      base = 16;
      break;

    case 'd':
      if (fmt[1] == 'E') {
        flags |= ERRSTR;
        fmt++;
      }
    /* fallthrough */
    case 'i':
      flags |= SIGN;
    /* fallthrough */
    case 'u':
      break;

    default:
      ADDCH(str, '%');
      if (*fmt)
        ADDCH(str, *fmt);
      else
        --fmt;
      continue;
    }
    if (qualifier == 'L') /* "quad" for 64 bit variables */
      num = va_arg(args, unsigned long long);
    else if (qualifier == 'l') {
      num = va_arg(args, unsigned long);
      if (flags & SIGN)
        num = (signed long)num;
    } else if (qualifier == 'Z' || qualifier == 'z') {
      num = va_arg(args, size_t);
    } else if (qualifier == 't') {
      num = va_arg(args, long);
    } else if (qualifier == 'h') {
      num = (unsigned short)va_arg(args, int);
      if (flags & SIGN)
        num = (signed short)num;
    } else {
      num = va_arg(args, unsigned int);
      if (flags & SIGN)
        num = (signed int)num;
    }
    str = number(str, end, num, base, field_width, precision, flags);
  }

  if (size > 0) {
    ADDCH(str, '\0');
    if (str > end)
      end[-1] = '\0';
    --str;
  }
  /* the trailing null byte doesn't count towards the total */
  return str - buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  return vsnprintf_internal(buf, size, fmt, args);
}

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  int i;

  i = vsnprintf(buf, size, fmt, args);

  if ((i < size))
    return i;
  if (size != 0)
    return size - 1;
  return 0;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsnprintf(buf, size, fmt, args);
  va_end(args);

  return i;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vscnprintf(buf, size, fmt, args);
  va_end(args);

  return i;
}

/**
 * Format a string and place it in a buffer (va_list version)
 *
 * @param buf	The buffer to place the result into
 * @param fmt	The format string to use
 * @param args	Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf. Use vsnprintf() or vscnprintf() in order to avoid
 * buffer overflows.
 *
 * If you're not already dealing with a va_list consider using sprintf().
 */
int vsprintf(char *buf, const char *fmt, va_list args) {
  return vsnprintf_internal(buf, INT_MAX, fmt, args);
}

int sprintf(char *buf, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);
  return i;
}

int printf(const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vprintf(fmt, args);
  va_end(args);

  return i;
}

int vprintf(const char *fmt, va_list args) {
  int i;
  char printbuffer[1024];

  /*
   * For this to work, printbuffer must be larger than
   * anything we ever want to print.
   */
  i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);

  /* Handle error */
  if (i <= 0)
    return i;
  /* Print the string */
  puts(printbuffer);
  return i;
}

static char local_toa[22];

char *simple_itoa(ulong i) {
  /* 21 digits plus null terminator, good for 64-bit or smaller ints */
  char *p = &local_toa[21];

  *p-- = '\0';
  do {
    *p-- = '0' + i % 10;
    i /= 10;
  } while (i > 0);
  return p + 1;
}

char *simple_xtoa(ulong num) {
  /* 16 digits plus nul terminator, good for 64-bit or smaller ints */
  char *p = &local_toa[17];

  *--p = '\0';
  do {
    p -= 2;
    hex_byte_pack(p, num & 0xff);
    num >>= 8;
  } while (num > 0);

  return p;
}

/* We don't seem to have %'d in U-Boot */
void print_grouped_ull(unsigned long long int_val, int digits) {
  char str[21], *s;
  int grab = 3;

  digits = (digits + 2) / 3;
  sprintf(str, "%*llu", digits * 3, int_val);
  for (s = str; *s; s += grab) {
    if (s != str)
      putc(s[-1] != ' ' ? ',' : ' ');
    printf("%.*s", grab, s);
    grab = 3;
  }
}

bool str2off(const char *p, loff_t *num) {
  char *endptr;

  *num = simple_strtoull(p, &endptr, 16);
  return *p != '\0' && *endptr == '\0';
}

bool str2long(const char *p, ulong *num) {
  char *endptr;

  *num = hextoul(p, &endptr);
  return *p != '\0' && *endptr == '\0';
}

char *strmhz(char *buf, unsigned long hz) {
  long l, n;
  long m;

  n = DIV_ROUND_CLOSEST(hz, 1000) / 1000L;
  l = sprintf(buf, "%ld", n);

  hz -= n * 1000000L;
  m = DIV_ROUND_CLOSEST(hz, 1000L);
  if (m != 0)
    sprintf(buf + l, ".%03ld", m);

  return buf;
}

#endif

/* Handy now and then */
void show_reg(char *msg, int *addr) {
  printf("%s %#llx %#llx\n", msg, (long)addr, *addr);

}
/* THE END */
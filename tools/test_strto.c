/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <errno.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;


#define _U 0x01 /* upper */
#define _L 0x02 /* lower */
#define _D 0x04 /* digit */
#define _C 0x08 /* cntrl */
#define _P 0x10 /* punct */
#define _S 0x20 /* white space (space/lf/tab) */
#define _X 0x40 /* hex digit */
#define _SP 0x80 /* hard space (0x20) */

const unsigned char
	_ctype[] = { _C,       _C,	_C,	 _C,	  _C,	   _C,
		     _C,       _C, /* 0-7 */
		     _C,       _C | _S, _C | _S, _C | _S, _C | _S, _C | _S,
		     _C,       _C, /* 8-15 */
		     _C,       _C,	_C,	 _C,	  _C,	   _C,
		     _C,       _C, /* 16-23 */
		     _C,       _C,	_C,	 _C,	  _C,	   _C,
		     _C,       _C, /* 24-31 */
		     _S | _SP, _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P, /* 32-39 */
		     _P,       _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P, /* 40-47 */
		     _D,       _D,	_D,	 _D,	  _D,	   _D,
		     _D,       _D, /* 48-55 */
		     _D,       _D,	_P,	 _P,	  _P,	   _P,
		     _P,       _P, /* 56-63 */
		     _P,       _U | _X, _U | _X, _U | _X, _U | _X, _U | _X,
		     _U | _X,  _U, /* 64-71 */
		     _U,       _U,	_U,	 _U,	  _U,	   _U,
		     _U,       _U, /* 72-79 */
		     _U,       _U,	_U,	 _U,	  _U,	   _U,
		     _U,       _U, /* 80-87 */
		     _U,       _U,	_U,	 _P,	  _P,	   _P,
		     _P,       _P, /* 88-95 */
		     _P,       _L | _X, _L | _X, _L | _X, _L | _X, _L | _X,
		     _L | _X,  _L, /* 96-103 */
		     _L,       _L,	_L,	 _L,	  _L,	   _L,
		     _L,       _L, /* 104-111 */
		     _L,       _L,	_L,	 _L,	  _L,	   _L,
		     _L,       _L, /* 112-119 */
		     _L,       _L,	_L,	 _P,	  _P,	   _P,
		     _P,       _C, /* 120-127 */
		     0,	       0,	0,	 0,	  0,	   0,
		     0,	       0,	0,	 0,	  0,	   0,
		     0,	       0,	0,	 0, /* 128-143 */
		     0,	       0,	0,	 0,	  0,	   0,
		     0,	       0,	0,	 0,	  0,	   0,
		     0,	       0,	0,	 0, /* 144-159 */
		     _S | _SP, _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P,	_P,	 _P, /* 160-175 */
		     _P,       _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P,	_P,	 _P,	  _P,	   _P,
		     _P,       _P,	_P,	 _P, /* 176-191 */
		     _U,       _U,	_U,	 _U,	  _U,	   _U,
		     _U,       _U,	_U,	 _U,	  _U,	   _U,
		     _U,       _U,	_U,	 _U, /* 192-207 */
		     _U,       _U,	_U,	 _U,	  _U,	   _U,
		     _U,       _P,	_U,	 _U,	  _U,	   _U,
		     _U,       _U,	_U,	 _L, /* 208-223 */
		     _L,       _L,	_L,	 _L,	  _L,	   _L,
		     _L,       _L,	_L,	 _L,	  _L,	   _L,
		     _L,       _L,	_L,	 _L, /* 224-239 */
		     _L,       _L,	_L,	 _L,	  _L,	   _L,
		     _L,       _P,	_L,	 _L,	  _L,	   _L,
		     _L,       _L,	_L,	 _L }; /* 240-255 */

extern const unsigned char _ctype[];

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

/*
 * Rather than doubling the size of the _ctype lookup table to hold a 'blank'
 * flag, just check for space or tab.
 */
#define isblank(c) (c == ' ' || c == '\t')

#define isascii(c) (((unsigned char)(c)) <= 0x7f)
#define toascii(c) (((unsigned char)(c)) & 0x7f)

// Function prototype
unsigned long long simple_strtoull(const char *cp, char **endp,
				   unsigned int base);

static inline unsigned char __tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A' - 'a';
	return c;
}

static inline unsigned char __toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a' - 'A';
	return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)


/* from lib/kstrtox.c */
static const char *_parse_integer_fixup_radix(const char *s, uint32_t *basep)
{
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
static uint32_t decode_digit(int ch)
{
	if (!isxdigit(ch))
		return 256;

	ch = tolower(ch);

	return ch <= '9' ? ch - '0' : ch - 'a' + 0xa;
}

ulong simple_strtoul(const char *cp, char **endp, uint32_t base)
{
	ulong result = 0;
	uint32_t value;

	cp = _parse_integer_fixup_radix(cp, &base);
	printf("cp:%s\n", cp);
	value = decode_digit(*cp);
	printf("*cp:%c val:%d\n", *cp, value);
	while (value < base) {
		result = result * base + value;
		cp++;
		value = decode_digit(*cp);
		printf("*cp:%c val:%d\n", *cp, value);
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

ulong hextoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 16);
}

ulong dectoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 10);
}

int strict_strtoul(const char *cp, unsigned int base, unsigned long *res)
{
	char *tail;
	unsigned long val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -EINVAL;

	val = simple_strtoul(cp, &tail, base);
	if (tail == cp)
		return -EINVAL;

	if ((*tail == '\0') ||
	    ((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -EINVAL;
}

long simple_strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoul(cp + 1, endp, base);

	return simple_strtoul(cp, endp, base);
}

unsigned long ustrtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = simple_strtoul(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = simple_strtoull(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long simple_strtoull(const char *cp, char **endp,
				   unsigned int base)
{
	unsigned long long result = 0;
	uint32_t value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (value = decode_digit(*cp), value < base) {
		result = result * base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

long long simple_strtoll(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoull(cp + 1, endp, base);

	return simple_strtoull(cp, endp, base);
}

long trailing_strtoln_end(const char *str, const char *end, char const **endp)
{
	const char *p;

	if (!end)
		end = str + strlen(str);
	p = end - 1;
	if (p > str && isdigit(*p)) {
		do {
			if (!isdigit(p[-1])) {
				if (endp)
					*endp = p;
				return dectoul(p, NULL);
			}
		} while (--p > str);
	}
	if (endp)
		*endp = end;

	return -1;
}

long trailing_strtoln(const char *str, const char *end)
{
	return trailing_strtoln_end(str, end, NULL);
}

long trailing_strtol(const char *str)
{
	return trailing_strtoln(str, NULL);
}

void str_to_upper(const char *in, char *out, size_t len)
{
	for (; len > 0 && *in; len--)
		*out++ = toupper(*in++);
	if (len)
		*out = '\0';
}

void str_free_list(const char **ptr)
{
	if (ptr)
		free((char *)ptr[0]);
	free(ptr);
}


int main(int argc, char *argv[])
{
	char *d = "0xfe660000";

	printf("%#llx\n", simple_strtoul(d, NULL, 16)); return 0;
}
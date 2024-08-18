
#ifndef _PRINTF_H
#define _PRINTF_H

int printf(const char *fmt, ...);
int scanf(const char * fmt, ...);
int printf_intr(const char *fmt, ...);
void closePrintf(void);
void openPrintf(void);

#endif /* _PRINTF_H */

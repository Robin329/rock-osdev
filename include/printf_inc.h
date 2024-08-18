#ifndef _PRINTF_INC_H
#define _PRINTF_INC_H


#include "printf.h"

#define TSENV	(0x1<<0) //test env log
#define ERROR	(0x1<<1) //error log
#define DEBUG	(0x1<<2) //debug log
#define MESG	(0x1<<3) //debug log

#define LOGON	(TSENV | ERROR | DEBUG)

#define hprintf(level, ...) do { \
	if(LOGON & level) { \
		printf(__VA_ARGS__); \
	} \
}while(0)

#endif

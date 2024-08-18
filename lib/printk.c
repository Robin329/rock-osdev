#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/printk.h>
#include <linux/string.h>
#include <vsprintf.h>
#include <common.h>

#define LOGLEVEL_DEFAULT '4'

u8 message_logleve = CONFIG_MESSAGE_LOGLEVEL_DEFAULT;

int printk(const char *fmt, ...)
{
	va_list args;
	int i;
	int kern_level;
	int level = LOGLEVEL_DEFAULT - '0';
	char log_buf[CONFIG_SYS_PBSIZE];
	char *buf = log_buf;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	while ((kern_level = printk_get_level(buf)) != 0) {
		switch (kern_level) {
		case '0' ... '7':
			level = kern_level - '0';
			/* fallthrough */
			break;
		case 'd': /* KERN_DEFAULT */
		default:
			level = LOGLEVEL_DEFAULT - '0';
			break;
		}
		i -= 2;
		buf += 2;
	}

	if (message_logleve >= level)
		puts(buf);

	return i;
}

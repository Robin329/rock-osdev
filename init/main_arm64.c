#include <common.h>
#include <cli.h>
#include <command.h>
#include <init.h>
#include <asm/mmu.h>
#include <time.h>
#include <linux/delay.h>

void main_loop(void)
{
	cli_init();
	cli_loop();
}
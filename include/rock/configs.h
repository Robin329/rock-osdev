#ifndef __CONFIGS_H_
#define __CONFIGS_H_

#include <linux/const.h>

#define CONFIG_NR_DRAM_BANKS 1

// #define CONFIG_START_ADDR_SP_SIZE (8 * 64 * 1024) /* 8 * 64KB */
// #define CONFIG_MALLOC_F_LEN       (64 * 1024 * 1024) /* 64MB */

#define CONFIG_SYS_INIT_SP_ADDR   (CONFIG_SYS_TEXT_BASE + 0xc000000)
#define CONFIG_START_ADDR_SP_SIZE (8 * 1024)
#define CONFIG_MALLOC_F_LEN       (64 * 1024)

#define CONFIG_SYS_HZ 1000
#define COUNTER_CLK_FREQ ULL(200000000) /* 200MHz */

#endif

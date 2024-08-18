#ifndef __MALLOC_H_
#define __MALLOC_H_

#include <configs.h>

#if (CONFIG_RELOC_SYS_BLOCK_SIZE <= 1024*1024)
#define PAGING_MEMORY (768*1024)
#else
#define PAGING_MEMORY CONFIG_MALLOC_F_LEN
#endif

#define PAGING_PAGES (PAGING_MEMORY >> 12)

int memory_init(void);
void *get_free_page(void);
void free_page(void *);

void *malloc(unsigned long len);
void free(void *obj);

#endif

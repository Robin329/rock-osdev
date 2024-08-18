#include <common.h>
#include <malloc.h>
#include <configs.h>

DECLARE_GLOBAL_DATA_PTR;

static int paging_pages = 0;

struct mem_map_struct {
    void *mem_addr;
    unsigned char use;
};

static struct mem_map_struct mem_map[PAGING_PAGES] = { 0, };

void *get_free_page(void)
{
    int i;

    for (i = 0; i < paging_pages; i++) {
        if (mem_map[i].use == 0) {
            mem_map[i].use++;
            return mem_map[i].mem_addr;
        }
    }

    return NULL;
}

void free_page(void *addr)
{
    unsigned long nr_addr = (unsigned long)addr;

    if (nr_addr < gd->malloc_base)
        return ;

    if (nr_addr >= gd->malloc_base + gd->malloc_limit)
        return ;

    if ((nr_addr & 0xfff)) {
        pr_warn("trying to free unalign page");
        return ;
    }

    nr_addr -= gd->malloc_base;
    nr_addr >>= 12;
    if ((mem_map[nr_addr].use - 1) != 0) {
        pr_emerg("trying to free busy or free page");
        return ;
    }

    mem_map[nr_addr].use--;
    memset(mem_map[nr_addr].mem_addr, 0, 4096);
}

int memory_init(void)
{
    int i;
    void *addr = (void *)gd->malloc_base;

    paging_pages = gd->malloc_limit >> 12;

    for (i = 0; i < paging_pages; i++) {
        mem_map[i].use = 0;
        mem_map[i].mem_addr = addr;
        addr += 4096;
    }
    printf("Memory %d page free (4KB)\n", paging_pages);
    return 0;
}

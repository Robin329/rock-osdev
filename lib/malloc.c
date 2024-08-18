#include <common.h>
#include <malloc.h>
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

struct bucket_desc {
	void *page;
	struct bucket_desc *next;
	void *freeptr;
	unsigned short refcnt;
	unsigned long bucket_size;
};

struct _bucket_dir {
	int size;
	struct bucket_desc *chain;
};

static struct _bucket_dir bucket_dir[] = {
	{ 16, (struct bucket_desc *)0 },   { 32, (struct bucket_desc *)0 },
	{ 64, (struct bucket_desc *)0 },   { 128, (struct bucket_desc *)0 },
	{ 256, (struct bucket_desc *)0 },  { 512, (struct bucket_desc *)0 },
	{ 1024, (struct bucket_desc *)0 }, { 2048, (struct bucket_desc *)0 },
	{ 4096, (struct bucket_desc *)0 }, { 0, (struct bucket_desc *)0 }
};

static struct bucket_desc *free_bucket_desc = (struct bucket_desc *)0;

static unsigned short bd_kmap[PAGING_PAGES] = { 0 };

static inline void init_bucket_desc(void)
{
	struct bucket_desc *bdesc, *first;
	int i;

	first = bdesc = (struct bucket_desc *)get_free_page();
	if (!bdesc)
		panic("Out of memory in init_bucket_desc()\n");

	for (i = PAGE_SIZE / sizeof(struct bucket_desc); i > 1; i--) {
		bdesc->next = bdesc + 1;
		bdesc++;
	}

	bdesc->next = free_bucket_desc;
	free_bucket_desc = first;
}

void *malloc(unsigned long len)
{
	int flag;
	struct _bucket_dir *bdir;
	struct bucket_desc *bdesc;
	void *retval = NULL;

	for (bdir = bucket_dir; bdir->size; bdir++)
		if (bdir->size >= len)
			break;

	if (!bdir->size) {
		pr_warn("malloc called with impossibly largument (%d)\n", len);
		return retval;
	}

	local_irq_save(flag);
	for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next)
		if (bdesc->freeptr)
			break;

	if (!bdesc) {
		char *cp;
		int i;

		if (!free_bucket_desc)
			init_bucket_desc();
		bdesc = free_bucket_desc;
		free_bucket_desc = bdesc->next;
		bdesc->refcnt = 0;
		bdesc->bucket_size = bdir->size;
		bdesc->page = bdesc->freeptr = (cp = get_free_page());
		if (!cp)
			panic("Out of memory in malloc()\n");

		for (i = PAGE_SIZE / bdir->size; i > 1; i--) {
			*((char **)cp) = cp + bdir->size;
			cp += bdir->size;
		}
		*((char **)cp) = 0;
		bdesc->next = bdir->chain;
		bdir->chain = bdesc;
		i = (unsigned long)bdesc->page - gd->malloc_base;
		bd_kmap[i >> 12] = bdir->size;
	}
	retval = (void *)bdesc->freeptr;
	bdesc->freeptr = *((void **)retval);
	bdesc->refcnt++;
	local_irq_restore(flag);

	return retval;
}

void free(void *obj)
{
	int flag;
	void *page, *freeptr;
	unsigned short size;
	struct _bucket_dir *bdir;
	struct bucket_desc *bdesc, *prev;

	page = (void *)((unsigned long)obj & (~4095));
	size = bd_kmap[((unsigned long)page - gd->malloc_base) >> 12];

	if ((unsigned long)obj & (size - 1))
		panic("Trying to free unalign addr\n");

	for (bdir = bucket_dir; bdir->size; bdir++) {
		prev = 0;

		if (bdir->size < size)
			continue;

		for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next) {
			if (bdesc->page == page)
				goto found;

			prev = bdesc;
		}
	}
	panic("Bad address passed to free()\n");

found:
	freeptr = (void *)bdesc->freeptr;
	if (freeptr != NULL) {
		while (*(void **)freeptr) {
			if (*(void **)freeptr == *(void **)obj)
				panic("Trying to repeat free addr\n");
			freeptr = *(void **)freeptr;
		}
	}

	local_irq_save(flag);
	*((void **)obj) = bdesc->freeptr;
	bdesc->freeptr = obj;
	bdesc->refcnt--;
	if (bdesc->refcnt == 0) {
		if ((prev && (prev->next != bdesc)) ||
		    (!prev && (bdir->chain != bdesc)))
			for (prev = bdir->chain; prev; prev = prev->next)
				if (prev->next == bdesc)
					break;

		if (prev)
			prev->next = bdesc->next;
		else {
			if (bdir->chain != bdesc)
				panic("Malloc bucket chains corrupted\n");
			bdir->chain = bdesc->next;
		}
		free_page(bdesc->page);
		bdesc->next = free_bucket_desc;
		free_bucket_desc = bdesc;
	}
	local_irq_restore(flag);
}

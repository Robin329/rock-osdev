CPPFLAGS_vmimage.lds := -DHEADDIR=arch/arm64/kernel

ifeq ($(CONFIG_DEBUG_MMU_CAHCE),y)
CFLAGS_cache_v8.o := -DDEBUG
endif

obj-y := cache.o cache_v8.o
obj-y += tlb.o transition.o
obj-y += crt0_64.o relocate_64.o sections.o
obj-y += interrupts_64.o exceptions.o
obj-y += generic_timer.o
obj-$(CONFIG_ARMV8_MULTIENTRY) += cpus.o
extra-y := start.o  vmimage.lds

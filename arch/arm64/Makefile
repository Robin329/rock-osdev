LDFLAGS_vmimage := --no-undefined -X

KBUILD_DEFCONFIG := defconfig

#KBUILD_CFLAGS	+= -mgeneral-regs-only $(lseinstr) $(brokengasinst)
KBUILD_CFLAGS	+= $(lseinstr) $(brokengasinst)
KBUILD_CFLAGS	+= -fno-asynchronous-unwind-tables

KBUILD_CFLAGS	+= $(call cc-option,-mabi=lp64)
KBUILD_AFLAGS	+= $(call cc-option,-mabi=lp64)

KBUILD_CFLAGS	+= -march=armv8.2-a
KBUILD_CFLAGS	+= -D__LINUX_ARM_ARCH__=8

ifeq ($(CONFIG_CPU_BIG_ENDIAN), y)
KBUILD_CPPFLAGS	+= -mbig-endian
CHECKFLAGS	+= -D__AARCH64EB__
AS		+= -EB
# Prefer the baremetal ELF build target, but not all toolchains include
# # it so fall back to the standard linux version if needed.
KBUILD_LDFLAGS	+= -EB $(call ld-option, -maarch64elfb, -maarch64linuxb)
UTS_MACHINE	:= aarch64_be
else
KBUILD_CPPFLAGS	+= -mlittle-endian
CHECKFLAGS	+= -D__AARCH64EL__
AS		+= -EL
# # Same as above, prefer ELF but fall back to linux target if needed.
KBUILD_LDFLAGS	+= -EL $(call ld-option, -maarch64elf, -maarch64linux)
UTS_MACHINE	:= aarch64
endif

# cancel built-in func
KBUILD_CFLAGS   += -fno-builtin

# by zy
KBUILD_CFLAGS += -ffunction-sections -fdata-sections \
		     -fno-common -ffixed-x18

# compiler speed by zy
KBUILD_CFLAGS += $(call cc-option,-pipe)

# needed for relocation by zy
LDFLAGS_vmimage += -pie

# by zy
LDFLAGS_vmimage += --gc-sections

CHECKFLAGS	+= -D__aarch64__
ARCH_CFLAGS += -DAARCH64

head-y :=

head-y := arch/arm64/kernel/start.o

core-y += arch/arm64/kernel/

boot := arch/arm64/boot

PHONY += $(BOARD).bin

all: $(BOARD).bin

KBUILD_IMAGE := $(BOARD).bin

$(BOARD).bin: vmimage
	$(Q)$(MAKE) $(build)=$(boot) $(boot)/$@

archscripts: scripts_basic

archheaders:

archprepare:

archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

define archhelp
	echo  '$(BOARD).bin   - kernel image (arch/$(ARCH)/boot/$(BOARD).bin)'
endef

OBJCOPYFLAGS_$(BOARD).bin := -O binary -R .comment -S


targets := $(BOARD).bin $(BOARD).lst vmimage System.map

# Debug maybe need lst file
OBJDUMPFLAGS_arm64.lst := -D -x -f

$(obj)/$(BOARD).bin:  vmimage $(obj)/arm64.lst FORCE
	$(call if_changed,objcopy)


$(obj)/arm64.lst: vmimage FORCE
	$(call if_changed,objdump)
	@cp vmimage arch/arm64/boot/
	@cp System.map arch/arm64/boot/

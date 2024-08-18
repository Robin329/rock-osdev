CFLAGS_command.o := -Wno-vla

ifeq ($(CONFIG_DEBUG_BOARD_INIT_F),y)
CFLAGS_board_f.o := -DDEBUG
endif

ifeq ($(CONFIG_DEBUG_BOARD_INIT_R),y)
CFLAGS_board_r.o := -DDEBUG
endif

ifeq ($(CONFIG_DEBUG_CLI),y)
CFLAGS_cli.o := -DDEBUG
CFLAGS_command.o += -DDEBUG
CFLAGS_cli_readline.o := -DDEBUG
CFLAGS_cli_simple.o := -DDEBUG
endif

ifeq ($(CONFIG_DEBUG_COMMON_MEMORY),y)
CFLAGS_memory.o := -DDEBUG
endif

obj-y := init/
obj-y += board_f.o board_r.o
obj-y += cli.o command.o
obj-$(CONFIG_CMDLINE) += cli_readline.o cli_simple.o
obj-y += memory.o interrupt.o cpu.o

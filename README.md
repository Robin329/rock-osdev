```sh
                    __
   _________  _____/ /______  _____
  / ___/ __ \/ ___/ //_/ __ \/ ___/
 / /  / /_/ / /__/ ,< / /_/ (__  )
/_/   \____/\___/_/|_|\____/____/
```
# ROCKOS
[![Build Status](https://github.com/Robin329/rock-osdev/actions/workflows/main.yml/badge.svg)](https://github.com/Robin329/rock-osdev/actions/workflows/main.yml)
## Introduce

This is a rockchip-based, not a completely bare drive. It is based on the uboot environment, which omits some unnecessary hardware initialization actions, such as DDR, CRM and other hardware resets. This OS can completely design your own OS in the bare drive way. Have fun;)

# Hardware Environment

```sh
Board: rock5a rk3588s
Compilation system:
Linux rock-5b 5.10.110-36-rockchip-gba392dc725fa #rockchip SMP Tue Jan 17 07:19:43 UTC 2023 aarch64 GNU/Linux
==========================================
No LSB modules are available.
Distributor ID: Debian
Description:    Debian GNU/Linux 11 (bullseye)
Release:        11
Codename:       bullseye
```

## Compile
### Clone code
```sh
$ git clone https://github.com/Robin329/rock-osdev.git

```

### Compile
```sh
$ cd rock-osdev
$ ./make.sh rock_defconfig
$ ./make.sh
```

## Run
First you need to enter uboot. This environment depends on tftp driver and network card driver. Please confirm whether these two environments are ok before burning.
```sh
=> setenv serverip 192.168.1.86;setenv ipaddr 192.168.1.102; tftpboot 0x02000000 rock.bin; go 0x2000000\n
```

```sh
=> setenv serverip 192.168.1.86;setenv ipaddr 192.168.1.102; tftpboot 0x02000000 rock.bin; go 0x02000000
ethernet@fe1c0000 Waiting for PHY auto negotiation to complete...... done
Using ethernet@fe1c0000 device
TFTP from server 192.168.1.86; our IP address is 192.168.1.102
Filename 'rock.bin'.
Load address: 0x2000000
Loading: ###
         7.8 KiB/s
done
Bytes transferred = 42048 (a440 hex)
## Starting application at 0x02000000 ...


                    __
   _________  _____/ /______  _____
  / ___/ __ \/ ___/ //_/ __ \/ ___/
 / /  / /_/ / /__/ ,< / /_/ (__  )
/_/   \____/\___/_/|_|\____/____/


Monitor len: 00061880
Ram size: 20000000
Ram top : 22000000
Reserving 390k for vmimage at: 21f9e000
Reserving 232 Bytes for Global Data at: 21f9df18
reserve_malloc:160 size:0x100000 start_addr_sp:0x21f9d000
Reserving 1024k for malloc at: 21e9d000

RAM Configuration:
Bank #0: 0 0 Bytes
DRAM:  0 Bytes

New Stack Pointer is: 21e9cff0
Relocation Offset is: 1ff9e000
Relocating to 21f9e000, new gd at 21f9df18, sp at 21e9cff0
Memory 256 page free (4KB)
irq_gicv3: Distributor has no Range Selector support, Support 512 irq
irq_gicv3: Gicv3 support MBIs, TODO MSI!
irq_gicv3: CPU0: GICR_TYPER:0x130001ef31
irq_gicv3: Support LPIs,TODO!

[rock]#
```
Have fun;)
#!/bin/bash

bear -- make ARCH=arm64 NO_OPTIMI=no CRYPTO=yes CROSS_COMPILE=aarch64-linux-gnu- $1 -j8
if [ $? -ne 0 ]; then
    echo "Make compile Error!"
    exit 1
fi
if [ x$1 == "x" ];then
    sudo cp arch/arm64/boot/rock.bin /home/rock/tftpboot/
fi
# aarch64-linux-gnu-objdump -axS arch/arm64/boot/vmimage > dump.txt
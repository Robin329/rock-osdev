#!/bin/bash

make ARCH=arm64 NO_OPTIMI=no CRYPTO=yes CROSS_COMPILE=aarch64-linux-gnu- $1 -j8
if [ $? -ne 0 ]; then
    echo "Make compile Error!"
    exit 1
fi
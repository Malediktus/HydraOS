#!/bin/bash

set -e

DIR="`dirname "${BASH_SOURCE[0]}"`"
cd "$DIR" || exit

source build.sh

qemu-system-x86_64 -drive file=../hydraos.img,format=raw -debugcon file:/dev/stdout -no-shutdown -no-reboot

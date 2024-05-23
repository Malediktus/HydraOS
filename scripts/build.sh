#!/bin/bash

set -e

DIR="`dirname "${BASH_SOURCE[0]}"`"
cd "$DIR" || exit

pushd ../kernel
    make build/kernel.elf
popd

mkdir -p /tmp/hydra_root/
mkdir -p /tmp/hydra_root/boot/grub
cp ../kernel/build/kernel.elf /tmp/hydra_root/boot/hydrakernel

cat > /tmp/hydra_root/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "HydraOS" {
	multiboot2 /boot/hydrakernel
	boot
}
EOF

grub-mkrescue -o ../hydraos.iso /tmp/hydra_root/

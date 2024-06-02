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
	multiboot2 /boot/hydrakernel klog=tty1
	boot
}
EOF

dd if=/dev/zero of=../hydraos.img bs=512 count=131072
fdisk ../hydraos.img << EOF
o
n
p
1


a
w
EOF

sudo losetup /dev/loop0 ../hydraos.img
sudo losetup /dev/loop1 ../hydraos.img -o 1048576

sudo mkdosfs -F32 -f 2 /dev/loop1
sudo mount /dev/loop1 /mnt
sudo cp -rf /tmp/hydra_root/* /mnt
sudo grub-install --root-directory=/mnt --no-floppy --modules="normal part_msdos multiboot2" /dev/loop0
sudo umount /mnt
sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1

# grub-mkrescue -o ../hydraos.iso /tmp/hydra_root/

#!/bin/bash

set -e

DIR="`dirname "${BASH_SOURCE[0]}"`"
cd "$DIR" || exit

rm -rf /tmp/hydra_root | true

mkdir -p /tmp/hydra_root/
mkdir -p /tmp/hydra_root/boot/grub
mkdir -p /tmp/hydra_root/bin
mkdir -p /tmp/hydra_root/lib
mkdir -p /tmp/hydra_root/include

pushd ../kernel
	echo "Compiling Kernel"
	make build/kernel.elf
	cp build/kernel.elf /tmp/hydra_root/boot/hydrakernel
	cp -r include/* /tmp/hydra_root/include/
popd

pushd ../libc
	echo "Compiling Libc"
	make build/libc.a
	cp build/libc.a /tmp/hydra_root/lib/libc.a
	cp -r include/* /tmp/hydra_root/include/
popd

for dir in ../apps/*/; do
	if [ -d "$dir" ]; then
		pushd $dir
			app=$(basename "$dir")
			echo "Compiling $app"
			make build/$app ROOT=/tmp/hydra_root/
			cp build/$app /tmp/hydra_root/bin/$app
		popd
	fi
done

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

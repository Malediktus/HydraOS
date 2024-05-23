#!/bin/bash

set -e

mkdir $HOME/src
pushd $HOME/src

git clone git://git.savannah.gnu.org/grub.git
cd grub
./bootstrap
cd ..
mkdir build_grub
cd build_grub
export AWK=gawk
../grub/configure --disable-werror TARGET_CC=x86_64-elf-gcc TARGET_OBJCOPY=x86_64-elf-objcopy \
TARGET_STRIP=x86_64-elf-strip TARGET_NM=x86_64-elf-nm TARGET_RANLIB=x86_64-elf-ranlib --target=x86_64-elf
make
sudo make install

popd
rm -rf $HOME/src

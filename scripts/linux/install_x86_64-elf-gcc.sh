#!/bin/bash

set -e

mkdir $HOME/src
pushd $HOME/src

export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

wget https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.xz
tar -xf gcc-14.1.0.tar.xz

mkdir -p build-gcc
cd build-gcc
../gcc-14.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-multilib

make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

popd
rm -rf $HOME/src

ROOT ?= ./

build/program: program.c
	mkdir -p build

	x86_64-elf-gcc -g -T ./linker.ld -o $@ -ffreestanding -O0 -nostdlib -fpic -g program.c $(ROOT)/lib/libc.a -I $(ROOT)/include -static -nostartfiles

.PHONY: all
all: build/program

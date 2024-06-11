build/program.bin: program.asm
	mkdir -p build

	nasm -f elf64 $^ -o ./build/program.o
	x86_64-elf-gcc -g -T ./linker.ld -o $@ -ffreestanding -O0 -nostdlib -fpic -g ./build/program.o

.PHONY: all
all: build/program.bin

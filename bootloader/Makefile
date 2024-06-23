BOBJS:=$(patsubst src/%.asm, build/%.asm.o, $(filter-out src/bootsector.asm, $(wildcard $(shell find src -name '*.asm'))))
BOBJS+=$(patsubst src/%.c, build/%.o, $(wildcard $(shell find src -name '*.c')))

ARCH:=x86_64

CFLAGS:=-Wall -Wextra -std=c99 -nostdlib -ffreestanding -O0 -g
ASFLAGS:=

CC:=$(ARCH)-elf-gcc
AS:=nasm
LD:=$(ARCH)-elf-ld

.PHONY: all
all: build/bootloader.bin build/bootsector.bin

build/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "CC\t$@"
	@$(CC) $(CFLAGS) -c $^ -o $@

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f elf64 -o $@ $^

build/bootloader.bin: $(BOBJS)
	@mkdir -p $(@D)
	@echo "LD\t$@"
	@$(LD) -o $@ -Ttext 0x7e00 $^ --oformat binary

build/bootsector.bin: src/bootsector.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f bin -o $@ $^

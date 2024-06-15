KOBJS:=$(patsubst src/%.c, build/%.o, $(wildcard $(shell find src -name '*.c')))
KOBJS+=$(patsubst src/%.asm, build/%.asm.o, $(wildcard $(shell find src -name '*.asm')))

ARCH:=x86_64

CFLAGS:=-Wall -Wextra -std=c99 -nostdlib -ffreestanding -O0 -g
ASFLAGS:=
LDFLAGS:=-n -m elf_$(ARCH) --no-dynamic-linker -nostdlib -z max-page-size=0x1000 --build-id=none -static

CC:=$(ARCH)-elf-gcc
AS:=nasm
LD:=$(ARCH)-elf-ld

.PHONY: all
all: build/kernel.elf

build/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "CC\t$@"
	@$(CC) $^ -o $@ -c $(CFLAGS) -Iinclude

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f elf64 -o $@ $^

build/kernel.elf: $(KOBJS)
	@mkdir -p $(@D)
	@echo "LD\t$@"
	@$(LD) $^ -o $@ -T src/arch/$(ARCH)/linker.ld $(LDFLAGS)
	@grub-file --is-x86-multiboot2 $@

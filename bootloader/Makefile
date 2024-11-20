ARCH:=x86_64

BLOADER_BIOS_OBJS := build/boot/bios/$(ARCH)/bootloader.asm.o

BLOADER_UEFI_OBJS := $(patsubst src/boot/uefi/%.asm, build/boot/uefi/%.asm.o, \
                     $(wildcard $(shell find src/boot/uefi -name '*.asm')))
BLOADER_UEFI_OBJS += $(patsubst src/boot/uefi/%.c, build/boot/uefi/%.o, \
                     $(wildcard $(shell find src/boot/uefi -name '*.c')))

BLOADER_OBJS := $(patsubst src/%.asm, build/%.asm.o, \
                $(wildcard $(shell find src -type f -name "*.asm" ! -path "src/boot/*")))
BLOADER_OBJS += $(patsubst src/%.c, build/%.o, \
                $(wildcard $(shell find src -type f -name "*.c" ! -path "src/boot/*")))


CFLAGS:=-Wall -Wextra -std=c99 -nostdlib -ffreestanding -O0 -g
ASFLAGS:=

CC:=$(ARCH)-elf-gcc
AS:=nasm
LD:=$(ARCH)-elf-ld

GNU_EFI:=../extern/gnu-efi

.PHONY: all
all: build/bootsector.bin build/bootloader.bin build/bootloader.elf build/bootloader.efi

build/bootloader.elf: $(BLOADER_OBJS)
	@mkdir -p $(@D)
	@echo "LD\t$@"
	@$(LD) $^ -o $@ -T src/arch/$(ARCH)/linker.ld -static -Bsymbolic -nostdlib

build/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "CC\t$@"
	@$(CC) $(CFLAGS) -c $^ -o $@

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f elf64 -o $@ $^

build/boot/bios/$(ARCH)/%.o: src/boot/bios/$(ARCH)/%.c
	@mkdir -p $(@D)
	@echo "CC\t$@"
	@$(CC) $(CFLAGS) -c $^ -o $@

build/boot/bios/$(ARCH)/%.asm.o: src/boot/bios/$(ARCH)/%.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f elf64 -o $@ $^

build/bootloader.bin: $(BLOADER_BIOS_OBJS) $(BLOADER_OBJS)
	@mkdir -p $(@D)
	@echo "LD\t$@"
	@$(LD) -o $@ -Ttext 0x7e00 $^ --oformat binary

build/bootloader.efi: src/boot/uefi/bootloader.c
	@mkdir -p $(@D)
	@echo "CC\tbuild/bootloader_efi.o"
	@x86_64-elf-gcc -Wa,--noexecstack -mno-red-zone -mno-avx -fpic -g -O0 -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -fno-stack-check -fno-merge-all-constants -Wno-error=unused-parameter -Wno-error=unused-variable -DCONFIG_x86_64 -DGNU_EFI_USE_MS_ABI -maccumulate-outgoing-args --std=c11 -I$(GNU_EFI)/inc -I$(GNU_EFI)/inc/$(ARCH) -I$(GNU_EFI)/inc/protocol -c -o build/bootloader_efi.o $^

	@echo "LD\tbuild/bootloader.efi.so"
	@x86_64-elf-ld -T$(GNU_EFI)/gnuefi/elf_$(ARCH)_efi.lds -nostdlib --warn-common --no-undefined --fatal-warnings --build-id=sha1 -shared -Bsymbolic -o build/bootloader.efi.so build/bootloader_efi.o $(GNU_EFI)/$(ARCH)/gnuefi/crt0-efi-x86_64.o -L$(GNU_EFI)/$(ARCH)/gnuefi -L$(GNU_EFI)/lib -lefi -lgnuefi
	
	@echo "OBJCPY\t$@"
	@objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/bootloader.efi.so $@

build/bootsector.bin: src/boot/bios/$(ARCH)/bootsector.asm
	@mkdir -p $(@D)
	@echo "AS\t$@"
	@nasm -f bin -o $@ $^

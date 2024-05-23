add-symbol-file kernel/build/kernel.elf
target remote | qemu-system-x86_64 -S -gdb stdio --no-shutdown hydraos.iso

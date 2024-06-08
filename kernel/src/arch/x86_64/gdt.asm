bits 64
global load_gdt

section .text

load_gdt:
    mov rax, rdi
    lgdt [rax]

    mov ax, 0x28
    ltr ax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push 0x08
    lea rax, [rel .next]
    push RAX
    retfq

.next:
    ret

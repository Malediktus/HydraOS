[bits 64]

section .text

global _start

_start:
    mov rax, 52
    syscall

label:
    jmp label

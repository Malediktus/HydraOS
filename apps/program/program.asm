[bits 64]

section .text

global syscall

syscall:
    mov rax, 52
    syscall
    ret

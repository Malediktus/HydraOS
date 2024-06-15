[bits 64]

section .text

global syscall

syscall:
    ; Arguments passed through registers:
    ; rdi -> syscall number
    ; rsi -> first argument
    ; rdx -> second argument
    ; r10 -> third argument
    ; r8  -> fourth argument
    ; r9  -> fifth argument

    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    mov r9, [rsp + 8]

    syscall

    ret

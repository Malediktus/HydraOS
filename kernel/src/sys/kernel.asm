global jump_usermode
extern test_user_function
jump_usermode:
    mov rcx, 0xc0000080 ; IA32_EFER
    rdmsr
    or rax, 1 ; set SCE (syscall extensions)
    wrmsr
    mov rcx, 0xc0000081 ; STAR
    rdmsr
    mov rdx, 0x00100008 ; kbase is 0x00, ubase is 0x10
    wrmsr

    mov rcx, 0x400000        ; first argument, new instruction pointer
    mov rsp, rbp        ; second argument, new stack pointer
    mov r11, 0x0200     ; eflags

    mov ax, 0x1b
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    o64 sysret;            ; to space!

global jump_usermode
extern syscall_handler_c

syscall_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11

    call syscall_handler_c

    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Return from the syscall
    sysret

jump_usermode:
;    mov rcx, 0xc0000080 ; IA32_EFER
;    rdmsr
;    or rax, 1 ; set SCE (syscall extensions)
;    wrmsr
;
;    mov rcx, 0xc0000081 ; STAR
;    rdmsr
;    mov rdx, 0x00100008
;    wrmsr
;
;    mov rcx, 0xc0000082      ; IA32_LSTAR
;    lea rax, [syscall_handler]
;    wrmsr
;
;    mov rcx, 0xc0000084      ; IA32_FMASK
;    mov rax, 0x200           ; disable interrupts (IF flag) during syscall
;    wrmsr

	mov rax, syscall_handler ; syscall handler entry
	mov	rdx, rax
	shr	rdx, 0x20

	mov rcx, 0xC0000082 ; LSTAR
	wrmsr
	mov	rcx, 0xc0000080 ; IA32_EFER
	rdmsr
	or eax, 1 ; set SCE (syscall extensions)
	wrmsr
	mov	rcx, 0xc0000081 ; STAR
	rdmsr
	mov	rdx, 0x00130008 ; syscall base is 0x08, sysret base is 0x13
	wrmsr
	mov	rcx, 0xc0000084 ; FMASK
	rdmsr
	mov	rax, (0xFFFFFFFFFFFFFFFF & ~0b10) ; mask every flag
	wrmsr

    mov rcx, 0x400000        ; first argument, new instruction pointer
    mov rsp, rbp        ; second argument, new stack pointer
    mov r11, 0x202     ; eflags

    o64 sysret;            ; to space!

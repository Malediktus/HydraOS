global task_execute
global syscall_init
extern syscall_handler
extern rsp0_stack

%define STACK_SIZE 4096*4

section .code

syscall_wrapper:
    cli
    mov [rsp_temp], rsp
    mov rsp, rsp0_stack + STACK_SIZE

    push qword [rsp_temp] ; rsp
    push qword rcx ; rip

    push qword rax
    push qword rbx
    push qword rcx
    push qword rdx
    push qword rbp
    push qword rdi
    push qword rsi
    push qword r8
    push qword r9
    push qword r10
    push qword r11
    push qword r12
    push qword r13
    push qword r14
    push qword r15

    ; syscall args
    push qword rsp
    push qword r9
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax
    mov r9, r8
    mov r8, r10

    call syscall_handler

    pop qword r9
    add rsp, 8

    pop qword r15
    pop qword r14
    pop qword r13
    pop qword r12
    pop qword r11
    pop qword r10
    pop qword r9
    pop qword r8
    pop qword rsi
    pop qword rdi
    pop qword rbp
    pop qword rdx
    pop qword rcx
    pop qword rbx
    pop qword rax

    pop qword rcx
    pop qword rsp

    o64 sysret

syscall_init:
	mov rax, syscall_wrapper ; syscall handler entry
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
    
    ret

task_execute:
    push qword 0x1b
    push qword rsi
    push qword rdx
    push qword 0x23
    push qword rdi

    mov rdi, rcx
    call task_restore_state

    iretq

task_restore_state:
    mov r15, [rdi]
    mov r14, [rdi+8]
    mov r13, [rdi+16]
    mov r12, [rdi+24]
    mov r11, [rdi+32]
    mov r10, [rdi+40]
    mov r9, [rdi+48]
    mov r8, [rdi+56]
    mov rsi, [rdi+64]
    mov rbp, [rdi+80]
    mov rdx, [rdi+88]
    mov rcx, [rdi+96]
    mov rbx, [rdi+104]
    mov rax, [rdi+112]

    mov rdi, [rdi+72]

    ret

section .bss

rsp_temp: resb 8

global task_execute
global syscall_init
extern syscall_handler
extern rsp0_stack

%define STACK_SIZE 4096*4

syscall_wrapper:
    cli
    mov rbp, rsp
    mov rsp, rsp0_stack + STACK_SIZE

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

    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax
    push qword r9
    mov r9, r8
    mov r8, r10

    call syscall_handler

    pop qword r9

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

    mov rsp, rbp

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
    mov r12, [rdi+16]
    mov r11, [rdi+24]
    mov r10, [rdi+32]
    mov r9, [rdi+40]
    mov r8, [rdi+48]
    mov rsi, [rdi+56]
    mov rbp, [rdi+72]
    mov rdx, [rdi+80]
    mov rcx, [rdi+88]
    mov rbx, [rdi+96]
    mov rax, [rdi+104]
    
    mov rdi, [rdi+64]

    ret

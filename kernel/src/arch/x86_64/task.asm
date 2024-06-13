global task_execute
global syscall_init
extern syscall_handler
extern rsp0_stack

%define STACK_SIZE 4096*4

syscall_wrapper:
    cli
    mov rbp, rsp
    mov rsp, rsp0_stack + STACK_SIZE

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

    mov rcx, r11 ; System V calling convention

    call syscall_handler

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
    mov rcx, 0x400000 ; instruction pointer
    mov rsp, 0x800000 ; new stack pointer
    mov r11, 0x202 ; eflags

    o64 sysret

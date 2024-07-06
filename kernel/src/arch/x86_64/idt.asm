bits 64
extern exception_handler
extern irq_handler

%macro pushad 0
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
%endmacro

%macro popad 0
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
%endmacro

isr_common_stub:
    pushad
    cld
    lea rdi, [rsp]
    call exception_handler
    popad
    add rsp, 0x10 
    iretq

%macro isr_err_stub 1
isr_stub_%+%1:
    cli
    push %1
    jmp isr_common_stub
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    cli
    push 0
    push %1
    jmp isr_common_stub
%endmacro

section .text

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_common_stub:
    pushad
    cld
    lea rdi, [rsp]
    call irq_handler
    popad
    add rsp, 0x10
    iretq

%assign i 32
%rep 	223
irq_stub_%+i:
    cli
    push 0
    push i
    jmp irq_common_stub
    %assign i i+1
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    32
    dq isr_stub_%+i 
    %assign i i+1 
%endrep

global irq_stub_table
irq_stub_table:
%assign i 32
%rep 	223
    dq irq_stub_%+i
    %assign i i+1
%endrep

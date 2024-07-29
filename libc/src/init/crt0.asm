bits 64
section .text

extern main
;extern exit
;extern initialize_standard_library

global _start
_start:
    mov rbp, 0
    push qword rbp
    push qword rbp
    mov qword rbp, rsp

    push qword rsi
    push qword rdi

	;call initialize_standard_library

    pop qword rdi
    pop qword rsi

	call main

    ;mov edi, eax
	;call exit

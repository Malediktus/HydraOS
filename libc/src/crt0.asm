section .text

;
; TODO: call global con- and destructors
;

; extern _init
; extern _fini
extern initialize_standard_library
extern main
extern exit
global _start

_start:
    ; Set up end of the stack frame linked list.
    mov rax, 0
    mov rbp, rax
    push rbp ; rip=0
    push rbp ; rbp=0
    mov rbp, rsp

    call initialize_standard_library

    ; Run the global constructors.
    ; call _init

    call main

    ; Run the global decstructors.
    ; call _fini

    ; Terminate the process with the exit code.
    mov edi, eax
    call exit

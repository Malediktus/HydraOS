bits 16
extern _start

%define DISK_ERR 1
%define NO_PCI_ERR 2
%define UNSUPPORTED_CPU_ERR 3
%define ENABLE_A20_ERR 4
%define MMAP_ERR 5

section .text

_start:
    mov [boot_drive], dl

    call check_pci
    cmp ax, 0x00
    je .no_pci

    call check_8086
    cmp ax, 0x00
    je .unsupported_cpu

    call check_cpuid
    cmp eax, 0x00
    je .unsupported_cpu

    call check_long_mode
    cmp ax, 0x00
    je .unsupported_cpu

    call check_msr
    cmp ax, 0x00
    je .unsupported_cpu

    call check_a20
    cmp ax, 0x00
    je .unsupported_cpu

    call enable_a20
    call get_mmap

    mov eax, 0xec00
    mov ebx, 0x0002
    int 0x15 ; tell BIOS where going to long mode

    call setup_page_tables
    call jump_long_mode

.no_pci:
    mov bl, NO_PCI_ERR
    call print_error_hlt

.unsupported_cpu:
    mov bl, UNSUPPORTED_CPU_ERR
    call print_error_hlt

;
; get_mmap
;  - return: none
;  - args: none
;
get_mmap:
    pusha
    clc

    int 0x12
    jc .error
    mov [low_memory], ax

    mov di, mmap
    mov ebx, 0x00
    mov edx, 0x534D4150

.loop:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    add word [mmap_length], 1

    jc .exit
    cmp ebx, 0x00
    je .exit

    add di, 24
    jmp .loop

.exit:
    popa
    ret

.error:
    mov bl, MMAP_ERR
    call print_error_hlt

;
; enable_a20
;  - return: none
;  - args: none
;
enable_a20:
    push ax

    mov     ax, 0x2402
    int     0x15
    jb      .error
    cmp     ah, 0
    jnz     .error

    cmp     al, 1
    jz      .exit

    mov     ax, 0x2401
    int     0x15
    jb      .error
    cmp     ah, 0
    jnz     .error

.exit:
    pop ax
    ret

.error:
    mov dl, ENABLE_A20_ERR
    call print_error_hlt

;
; check_a20
;  - return: ax is 0 when not available
;  - args: none
;
check_a20:
    mov ax, 0x2403
    int 0x15

    jb .no_a20
    cmp ah,0
    jnz .no_a20

    mov ax, 1
    ret

.no_a20:
    mov ax, 0
    ret

;
; check_msr
;  - return: ax is 0 when not available
;  - args: none
;
check_msr:
    push dx
    push cx

    mov eax, 1
    xor ecx, ecx
    cpuid

    test edx, 1 << 5
    jz .no_msr

    mov ax, 1
    pop cx
    pop dx
    ret

.no_msr:
    mov ax, 0
    pop cx
    pop dx
    ret

;
; check_long_mode
;  - return: ax is 0 when not available
;  - args: none
;
check_long_mode:
    push dx

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode

    mov ax, 1
    pop dx
    ret

.no_long_mode:
    mov ax, 0
    pop dx
    ret
;
; check_cpuid
;  - return: eax is 0 when not available
;  - args: none
;
check_cpuid:
    pushfd
    pushfd
    xor dword [esp], 0x00200000
    popfd
    pushfd
    pop eax
    xor eax,[esp]
    popfd
    and eax, 0x00200000

    ret

;
; check_8086
;  - return: ax is 0 when not available
;  - args: none
;
check_8086:
    push bx

    ; push flags and modify bits 12-15
    pushf
    pop bx
    mov ax, bx
    and bh, 0x0f
    push bx
    popf

    ; check if bits are still set
    pushf
    pop bx
    and bh, 0xf0
    cmp bh, 0xf0
    push ax
    popf

    jz .no_8086

    mov ax, 1
    pop bx
    ret

.no_8086:
    mov ax, 0
    pop bx
    ret

;
; check_pci
;  - return: ax is 0 when not available
;  - args: none
;
check_pci:
    mov ax, 0xb101
    int 0x1a

    cmp ah, 0x00
    jne .no_pci

    mov ax, 1
    ret

.no_pci:
    mov ax, 0
    ret

;
; print_error_hlt
;  - return: halts
;  - args: bl -> error code
;
print_error_hlt:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10

    mov ah, 0x0e
    add bl, 0x30
    mov al, bl
    int 0x10

    cli
    hlt

;
; setup_page_tables
;  - return: none
;  - args: none
;
setup_page_tables:
    pusha

    mov eax, page_table_l3
    or eax, 0b11
    mov [page_table_l4], eax
    
    mov eax, page_table_l2
    or eax, 0b11
    mov [page_table_l3], eax
    
    xor ecx, ecx

.loop:
    mov eax, 0x200000
    imul eax, ecx
    or eax, 0b10000011
    mov [page_table_l2 + ecx*8], eax
    
    inc ecx
    cmp ecx, 512
    jne .loop

    popa
    ret

jump_long_mode:
    cli

    mov eax, page_table_l4
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31 | 1 << 0
    mov cr0, eax

    lgdt [gdt.ptr]

    mov ax, gdt.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp gdt.code:long_mode_start

section .data

boot_drive: db 0x00

section .rodata

PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0
 
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

gdt:
    .null: equ $ - gdt
        dq 0
    .code: equ $ - gdt
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | EXEC | RW            ; Access
        db GRAN_4K | LONG_MODE | 0xF                ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .data: equ $ - gdt
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | RW                   ; Access
        db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .ptr:
        dw $ - gdt - 1
        dq gdt

section .bss

align 4096
page_table_l4 resb 4096

align 4096
page_table_l3 resb 4096

align 4096
page_table_l2 resb 4096

low_memory resw 1
mmap_length resw 1
mmap resb 480 ; 20 * 24

boot_info resb 19

section .code

; ==============================
; ========== LONG MODE =========
; ==============================

bits 64

extern main

long_mode_start:
    mov byte [boot_info], 0 ; legacy BIOS
    
    mov ax, [low_memory]
    mov word [boot_info+1], ax

    mov qword [boot_info+3], mmap

    xor rax, rax
    mov ax, [mmap_length]
    mov qword [boot_info+11], rax

    mov rdi, boot_info
    call main

    cli
    hlt

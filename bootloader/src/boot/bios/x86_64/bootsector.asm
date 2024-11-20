bits 16
org 0x7c00

%define SECOND_STAGE_ADDR 0x7e00
%define SECOND_STAGE_SECTORS 125

%define NO_EXTENDED_MODE_ERR 0
%define DISK_ERR 1

jmp setup

setup:
    mov [boot_drive], dl

    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov bp, 0x7c00
    mov sp, bp

entry:
    call clear_screen

    call check_extended_mode
    jc .no_extended_mode

    mov ax, SECOND_STAGE_SECTORS
    mov si, SECOND_STAGE_ADDR
    mov bx, 0x01
    xor cx, cx
    call disk_read

    mov dl, [boot_drive]

    jmp 0x0000:SECOND_STAGE_ADDR

.no_extended_mode:
    mov dl, NO_EXTENDED_MODE_ERR
    call print_error_hlt

;
; print_error_hlt
;  - return: halts
;  - args: bl -> error code
;
print_error_hlt:
    xor bx, bx

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
; clear_screen
;  - return: none
;  - args: none
;
clear_screen:
    pusha

    mov ax, 0x0700
    mov bh, 0x07
    mov cx, 0x0000
    mov dx, 0x184f
    int 0x10

    mov ah, 0x02
    mov bh, 0x00
    mov dx, 0x00
    int 0x10

    popa
    ret

;
; disk_read
;  - return: none
;  - args: ax -> num_blocks, ds:si -> transfer buffer, bx -> lba_low, cx -> lba_hi
;
disk_read:
    pusha

    mov [disk_address_packet.num_blocks], ax
    mov [disk_address_packet.transfer_buffer_seg], ds
    mov [disk_address_packet.transfer_buffer_off], si
    mov [disk_address_packet.lba_low], bx
    mov [disk_address_packet.lba_hi], cx
    
    xor ax, ax
    mov ds, ax

    mov si, disk_address_packet
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13

    jc .error

    popa
    ret

.error:
    mov dl, DISK_ERR
    call print_error_hlt

disk_address_packet:
    .size: db 0x10 ; 16 bytes
    .reserved1: db 0x00
    .num_blocks: dw 0x0000
    .transfer_buffer_off: dw 0x0000
    .transfer_buffer_seg: dw 0x0000
    .lba_low: dw 0x0000
    .lba_hi: dw 0x0000
    .reserved2: dd 0x000000

;
; check_extended_mode
;  - return: carry flag is set when not available
;  - args: none
;
check_extended_mode:
    pusha

    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, [boot_drive]
    int 0x13

    popa
    ret

boot_drive: db 0x00

times 510-($-$$) db 0
dw 0xaa55

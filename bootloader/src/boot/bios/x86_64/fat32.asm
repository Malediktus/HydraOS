bits 16
section .text

read_mbr:
    pusha

    mov ax, 1
    mov si, 0x7c00
    mov bx, 1
    call disk_read

    mov si, 0x7c00 + 0x1be

.loop:
    cmp si, 0x1ee
    jge .exit

    mov ax, si
    add ax, 0x04
    
    cmp [ax], 0
    je .continue

    add ax, 0x04
    mov [fat32_lba], [ax]

    jmp .exit

.continue:
    add si, 16
    jmp .loop

.exit:
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

section .data

fat32_lba: dd 0x00

disk_address_packet:
    .size: db 0x10 ; 16 bytes
    .reserved1: db 0x00
    .num_blocks: dw 0x0000
    .transfer_buffer_off: dw 0x0000
    .transfer_buffer_seg: dw 0x0000
    .lba_low: dw 0x0000
    .lba_hi: dw 0x0000
    .reserved2: dd 0x000000

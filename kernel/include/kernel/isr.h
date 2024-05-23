#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <stdint.h>
#include <stddef.h>

void enable_interrupts(void);
void disable_interrupts(void);

typedef struct
{
    uint64_t r11, r10, r9, r8;
    uint64_t rsi, rdi, rdx, rcx, rax;
    uint64_t int_no, err_code;
    uint64_t rsp, rflags, cs, rip;
} __attribute__((packed)) interrupt_frame_t;

int register_interrupt_handler(uint8_t irq, void (*handler)(interrupt_frame_t *));
int interrupts_init(void);

#endif
#ifndef _KERNEL_PIT_H
#define _KERNEL_PIT_H

#include <stdint.h>
#include <kernel/isr.h>

int pit_init(uint32_t frequency);
void pit_set_frequency(uint32_t frequency);
uint32_t pit_get_frequency(void);
void register_pit_handler(void (*func)(interrupt_frame_t *frame, uint32_t frequency));

void sleep(uint64_t ms);

#endif
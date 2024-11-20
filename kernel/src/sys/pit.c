#include <kernel/pit.h>
#include <kernel/port.h>

#define MAX_PIT_HANDLERS 255

void (*pit_handlers[MAX_PIT_HANDLERS])(interrupt_frame_t *frame, uint32_t frequency);
uint8_t num_pit_handlers = 0;

static uint32_t _frequency;
static uint64_t sleep_ticks = 0;

static void timer_irq(interrupt_frame_t *frame)
{
    for (uint8_t i = 0; i < num_pit_handlers; i++)
    {
        pit_handlers[i](frame, _frequency);
    }

    sleep_ticks++;
}

int pit_init(uint32_t frequency)
{
    pit_set_frequency(frequency);
    return register_interrupt_handler(0x20, &timer_irq);
}

void pit_set_frequency(uint32_t frequency)
{
    uint32_t divisor = 1193180 / frequency;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);

    _frequency = frequency;
}

uint32_t pit_get_frequency(void)
{
    return _frequency;
}

void register_pit_handler(void (*func)(interrupt_frame_t *frame, uint32_t frequency))
{
    pit_handlers[num_pit_handlers++] = func;
}

void sleep(uint64_t ms)
{
    uint64_t ticks_needed = (_frequency * ms) / 1000;
    sleep_ticks = 0;

    while (sleep_ticks < ticks_needed);
}

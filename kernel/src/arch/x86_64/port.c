#include <kernel/port.h>

uint8_t port_byte_in(uint16_t port)
{
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data)
{
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

uint16_t port_word_in(uint16_t port)
{
    uint16_t result;
    __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data)
{
    __asm__("out %%ax, %%dx" : : "a"(data), "d"(port));
}

uint32_t port_dword_in(uint16_t port)
{
    uint32_t result;
    __asm__("in %%dx, %%eax" : "=a"(result) : "d"(port));
    return result;
}

void port_dword_out(uint16_t port, uint32_t data)
{
    __asm__("out %%eax, %%dx" : : "a"(data), "d"(port));
}

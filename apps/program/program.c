#include <h_syscall.h>
#include <stddef.h>

void read(uint64_t stream, uint32_t *buf, size_t len)
{
    syscall(0, stream, (uint64_t)buf, len, 0, 0, 0); // TODO: color/format info on second byte
}

void write(uint64_t stream, const uint32_t *buf, size_t len)
{
    syscall(1, stream, (uint64_t)buf, len, 0, 0, 0);
}

uint64_t fork(void)
{
    return syscall(2, 0, 0, 0, 0, 0, 0);
}

void *allocate_page(void)
{
    return (void *)syscall(3, 0, 0, 0, 0, 0, 0);
}

#define MODIFIER_SHIFT 1 << 0
#define MODIFIER_CTRL 1 << 1
#define MODIFIER_ALT 1 << 2
#define MODIFIER_CAPS_LOCK 1 << 3

// Define the QWERTZ layout mappings
const char qwertz_normal[128] = {
    /* 0x00 */ 0, 0, '1', '2', '3', '4', '5', '6',
    /* 0x08 */ '7', '8', '9', '0', 0, 0, '\b', 0,
    /* 0x10 */ 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i',
    /* 0x18 */ 'o', 'p', 0, '+', '\n', 0, 'a', 's',
    /* 0x20 */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0,
    /* 0x28 */ 0, '#', 'y', 'x', 'z', 'v', 'b', 'n',
    /* 0x30 */ 'm', ',', 0, '-', '.', 0, 0, ' ',
    // Other scancodes mapped to 0 or unused
};

const char qwertz_shift[128] = {
    /* 0x00 */ 0, 0, '!', '"', 0, '$', '%', '&',
    /* 0x08 */ '/', '(', ')', '=', 0, 0, '\b', 0,
    /* 0x10 */ 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I',
    /* 0x18 */ 'O', 'P', 0, '*', '\n', 0, 'A', 'S',
    /* 0x20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0,
    /* 0x28 */ 0, '\'', 'Y', 'X', 'Z', 'V', 'B', 'N',
    /* 0x30 */ 'M', ';', 0, '_', ':', 0, 0, ' ',
    // Other scancodes mapped to 0 or unused
};

// Caps Lock affects only alphabetic characters
const char qwertz_caps[128] = {
    /* 0x00 */ 0, 0, '1', '2', '3', '4', '5', '6',
    /* 0x08 */ '7', '8', '9', '0', 0, 0, '\b', 0,
    /* 0x10 */ 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I',
    /* 0x18 */ 'O', 'P', 0, '+', '\n', 0, 'A', 'S',
    /* 0x20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0,
    /* 0x28 */ 0, '#', 'Y', 'X', 'Z', 'V', 'B', 'N',
    /* 0x30 */ 'M', ',', 0, '-', '.', 0, 0, ' ',
    // Other scancodes mapped to 0 or unused
};

// Implementation of the function
char scancode_to_ascii(uint8_t scancode, uint8_t modifier)
{
    if (scancode >= 128)
    {
        return 0; // Invalid scancode
    }

    char ascii_char;

    // Check if Shift or Caps Lock is active
    if ((modifier & MODIFIER_SHIFT) && (modifier & MODIFIER_CAPS_LOCK))
    {
        // Both Shift and Caps Lock active
        ascii_char = qwertz_normal[scancode];
        if (ascii_char >= 'a' && ascii_char <= 'z')
        {
            ascii_char -= 32; // Convert to uppercase
        }
        else if (ascii_char >= 'A' && ascii_char <= 'Z')
        {
            ascii_char += 32; // Convert to lowercase
        }
    }
    else if (modifier & MODIFIER_SHIFT)
    {
        ascii_char = qwertz_shift[scancode];
    }
    else if (modifier & MODIFIER_CAPS_LOCK)
    {
        ascii_char = qwertz_caps[scancode];
    }
    else
    {
        ascii_char = qwertz_normal[scancode];
    }

    // Additional handling for Ctrl, Alt can be added if necessary
    if (modifier & MODIFIER_CTRL)
    {
        // Implement Ctrl-specific behavior if needed
    }
    if (modifier & MODIFIER_ALT)
    {
        // Implement Alt-specific behavior if needed
    }

    return ascii_char;
}

void memcpy(void *dest, const void *src, size_t len)
{
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for (size_t i = 0; i < len; i++)
    {
        cdest[i] = csrc[i];
    }
}

void *memset(void *dest, register int val, register size_t len)
{
    register unsigned char *ptr = (unsigned char *)dest;
    while (len-- > 0)
    {
        *ptr++ = val;
    }
    return dest;
}

int main(void)
{
    const uint32_t *str = L"Hello User World!\n";
    write(1, str, 18);

    uint64_t pid = fork();
    if (pid == 0)
    {
        write(1, L"Forked process\n", 15);
        while (1);
    }

    uint32_t c = 0;
    while (1)
    {
        read(0, &c, 1);
        if (c != 0)
        {
            uint8_t packet[2];
            memcpy(packet, &c, 2);
            uint32_t ascii = (uint32_t)scancode_to_ascii(packet[0], packet[1]);
            write(1, &ascii, 1);
        }
    }

    while (1);
    return 0;
}

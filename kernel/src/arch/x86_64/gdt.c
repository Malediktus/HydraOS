#include <kernel/smm.h>
#include <kernel/string.h>
#include <stdint.h>
#include <stddef.h>

#define ACCESS_PRESENT 0x80              // 10000000
#define ACCESS_PRIVILEGE_RING0 0x00      // 00000000
#define ACCESS_PRIVILEGE_RING1 0x20      // 00100000
#define ACCESS_PRIVILEGE_RING2 0x40      // 01000000
#define ACCESS_PRIVILEGE_RING3 0x60      // 01100000
#define ACCESS_SEGMENT 0x10              // 00010000
#define ACCESS_EXECUTABLE 0x08           // 00001000
#define ACCESS_DIRECTION_CONFORMING 0x04 // 00000100
#define ACCESS_READ_WRITE 0x02           // 00000010
#define ACCESS_ACCESSED 0x01             // 00000001

#define FLAG_GRANULARITY 0x8 // 00001000
#define FLAG_SIZE 0x4        // 00000100
#define FLAG_LONG_MODE 0x2   // 00000010
#define FLAG_AVAILABLE 0x1   // 00000001

typedef struct
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t flags_limit1;
    uint8_t base2;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdt_ptr_t;

static void populate_gdt_entry(gdt_entry_t *entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    uint8_t *raw = (uint8_t *)entry;
    raw[0] = limit & 0xFF;
    raw[1] = (limit >> 8) & 0xFF;
    raw[6] = (limit >> 16) & 0xFF;

    raw[2] = base & 0xFF;
    raw[3] = (base >> 8) & 0xFF;
    raw[4] = (base >> 16) & 0xFF;
    raw[7] = (base >> 24) & 0xFF;

    raw[5] = access;

    raw[6] |= (flags << 4);
}

__attribute((aligned(0x1000))) gdt_entry_t gdt[5];
__attribute((aligned(0x1000))) gdt_ptr_t gdt_ptr;

extern void load_gdt(gdt_ptr_t *); // defined int gdt.asm

int segmentation_init(void)
{
    memset(gdt, 0, sizeof(gdt));
    populate_gdt_entry(&gdt[0], 0, 0, 0, 0);
    populate_gdt_entry(&gdt[1], 0, 0xFFFFF, ACCESS_PRESENT | ACCESS_PRIVILEGE_RING0 | ACCESS_SEGMENT | ACCESS_READ_WRITE | ACCESS_EXECUTABLE, FLAG_LONG_MODE | FLAG_GRANULARITY);
    populate_gdt_entry(&gdt[2], 0, 0xFFFFF, ACCESS_PRESENT | ACCESS_PRIVILEGE_RING0 | ACCESS_SEGMENT | ACCESS_READ_WRITE, FLAG_SIZE | FLAG_GRANULARITY);
    populate_gdt_entry(&gdt[3], 0, 0xFFFFF, ACCESS_PRESENT | ACCESS_PRIVILEGE_RING3 | ACCESS_SEGMENT | ACCESS_READ_WRITE | ACCESS_EXECUTABLE, FLAG_LONG_MODE | FLAG_GRANULARITY);
    populate_gdt_entry(&gdt[4], 0, 0xFFFFF, ACCESS_PRESENT | ACCESS_PRIVILEGE_RING3 | ACCESS_SEGMENT | ACCESS_READ_WRITE, FLAG_SIZE | FLAG_GRANULARITY);

    gdt_ptr.size = (uint16_t)sizeof(gdt) - 1;
    gdt_ptr.offset = (uint64_t)gdt;

    load_gdt(&gdt_ptr);

    return 0;
}

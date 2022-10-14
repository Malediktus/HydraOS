#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/config.h>
#include <kernel/memory/kheap.h>

struct gdt gdt_real[KERNEL_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[KERNEL_TOTAL_GDT_SEGMENTS] = {
	{.base = 0x00, .limit = 0x00, .type = 0x00},	   // NULL Segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0x9a}, // Kernel code segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0x92}, // Kernel data segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0xf8}, // User code segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, // User data segment
													   // {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} // TSS Segment
};

void kernel_main(void)
{
	terminal_initialize();
	printf("Started kernel init!\n");

	memset(gdt_real, 0x00, sizeof(gdt_real));
	gdt_structured_to_gdt(gdt_real, gdt_structured, KERNEL_TOTAL_GDT_SEGMENTS);

	// Load the gdt
	gdt_load(gdt_real, sizeof(gdt_real));

	// Initialize the heap
	kheap_init();

	void *ptr1 = malloc(512);
	void *ptr2 = malloc(1024);
	free(ptr1);
	void *ptr3 = malloc(512);

	printf("1: %d, 2: %d, 3: %d\n", (int)ptr1, (int)ptr2, (int)ptr3);

	printf("Kernel init finished!\n");
}

#ifndef _KERNEL_VMM_H
#define _KERNEL_VMM_H

#include <stddef.h>
#include <stdbool.h>
#include <kernel/pmm.h>

#define PAGE_PRESENT 0x1                 // Present
#define PAGE_WRITABLE 0x2                // Writable
#define PAGE_USER 0x4                    // User/Supervisor (0: Supervisor, 1: User)
#define PAGE_WRITETHRU 0x8               // Write-Through
#define PAGE_NOT_CACHE 0x10              // Cache Disabled
#define PAGE_ACCESSED 0x20               // Accessed
#define PAGE_DIRTY 0x40                  // Dirty
#define PAGE_HUGE 0x80                   // Huge Page (1 GB or 2 MB pages)
#define PAGE_GLOBAL 0x100                // Global Page
#define PAGE_NO_EXECUTE 0x80000000000000 // No Execute (NX) bit

typedef struct page_table
{
    uint64_t entries[512];
} page_table_t;

// WARNING: the pml4 must always be page aligned
// pml is the virtual address to the pml4
int pml4_map(page_table_t *pml4, void *virt, void *phys, uint64_t flags);
int pml4_map_range(page_table_t *pml4, void *virt, void *phys, size_t num, uint64_t flags);
uint64_t pml4_get_phys(page_table_t *pml4, void *virt, bool user);

// WARNING: pml4 needs to be a physical address
int pml4_switch(page_table_t *pml4);

#endif
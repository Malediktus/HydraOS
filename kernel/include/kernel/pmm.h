#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

#define MMAP_ENTRY_TYPE_AVAILABLE 1<<0
#define MMAP_ENTRY_TYPE_RESERVED 1<<1
#define MMAP_ENTRY_TYPE_ACPI_RECLAIMABLE 1<<2

typedef struct
{
    uint64_t addr;
    uint64_t size;
    uint8_t type;
} memory_map_entry_t;

int pmm_init(memory_map_entry_t *memory_map, uint64_t num_mmap_entries, uint64_t total_memory);
void *pmm_alloc(void);
void pmm_free(uint64_t *page);

uint64_t get_max_addr(void);

#endif
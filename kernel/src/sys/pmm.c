#include <kernel/pmm.h>
#include <stddef.h>

#include <kernel/kprintf.h>

uint64_t *free_list_start = NULL;
uint64_t *free_list_end = NULL;

int pmm_init(memory_map_entry_t *memory_map, uint64_t num_mmap_entries)
{
    if (!memory_map || num_mmap_entries <= 0)
    {
        return -1;
    }

    for (uint64_t i = 0; i < num_mmap_entries; i++)
    {
        if (memory_map[i].type != MMAP_ENTRY_TYPE_AVAILABLE)
        {
            continue;
        }

        uint64_t base = ((memory_map[i].addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
        for (uint32_t j = 0; j < memory_map[i].size / PAGE_SIZE; j++)
        {
            if (free_list_start == NULL)
            {
                free_list_start = (uint64_t*)(base + j * PAGE_SIZE);
                *free_list_start = 0;
                continue;
            }

            uint64_t *ptr = free_list_start;
            for (; *ptr != 0; ptr = (uint64_t*)*ptr);
            *ptr = (base + j * PAGE_SIZE);
            *(uint64_t *)(base + j * PAGE_SIZE) = 0;
        }
    }

    return 0;
}

void *pmm_alloc(void)
{
    void *res = free_list_start;
    free_list_start = (uint64_t*)*free_list_start;
    return res;
}

void pmm_free(uint64_t *page)
{
    if (page < free_list_start)
    {
        *page = (uint64_t)free_list_start;
        free_list_start = page;
        return;
    }

    uint64_t *ptr = free_list_start;
    for (; (uintptr_t)ptr < (uintptr_t)page && *ptr > (uintptr_t)page; ptr = (uint64_t*)*ptr);
    *page = *ptr;
    *ptr = (uint64_t)page;
}

int pmm_reserve(uint64_t *addr)
{
    if (addr == free_list_start)
    {
        free_list_start = (uint64_t *)*free_list_start;
        return 0;
    }

    uint64_t *ptr = free_list_start;
    for (; (uint64_t)ptr != *addr && *ptr != 0; ptr = (uint64_t*)*ptr);
    if (*ptr == 0)
    {
        return -1;
    }

    *ptr = *addr;
    return 0;
}

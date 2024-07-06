#ifndef _KERNEL_KMM_H
#define _KERNEL_KMM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/vmm.h>

int kmm_init(page_table_t *kernel_pml4, uint64_t base, size_t size, size_t alignment);
void *kmalloc(size_t size);
void kfree(void *ptr);

void *krealloc(void *ptr, size_t old_size, size_t new_size);

#endif
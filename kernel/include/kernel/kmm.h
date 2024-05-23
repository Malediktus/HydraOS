#ifndef _KERNEL_KMM_H
#define _KERNEL_KMM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int kmm_init(void *arena, size_t size, size_t alignment);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
#ifndef _KERNEL_KHEAP_H
#define _KERNEL_KHEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/memory/heap.h>

void kheap_init();
void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *ptr);

#endif
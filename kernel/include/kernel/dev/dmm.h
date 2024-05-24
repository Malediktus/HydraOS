#ifndef _KERNEL_DMM_H
#define _KERNEL_DMM_H

#include <kernel/dev/chardev.h>
#include <stddef.h>

// TODO: use dynamic memory allocation
// TODO: use pci enumeration for char devices

int init_devices(void);
void free_devices(void);

chardev_t *get_chardev(size_t index);

#endif
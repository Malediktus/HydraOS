#ifndef _KERNEL_DMM_H
#define _KERNEL_DMM_H

#include <kernel/dev/chardev.h>
#include <kernel/dev/blockdev.h>
#include <stddef.h>

int init_devices(void);
void free_devices(void);

chardev_t *get_chardev(size_t index);
blockdev_t *get_blockdev(size_t index);

#endif
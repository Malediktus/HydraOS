#ifndef _KERNEL_SMM_H
#define _KERNEL_SMM_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

int segmentation_init(void);

#endif
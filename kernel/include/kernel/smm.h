#ifndef _KERNEL_SMM_H
#define _KERNEL_SMM_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR 0x1B
#define USER_DATA_SELECTOR 0x23

int segmentation_init(void);

#endif
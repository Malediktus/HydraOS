#ifndef _KERNEL_BLOCKDEV_H
#define _KERNEL_BLOCKDEV_H

#include <stdint.h>
#include <stddef.h>

typedef struct _blockdev
{
    int (*free)(struct _blockdev *);
    int (*read_block)(uint64_t, uint8_t *, struct _blockdev *);
    int (*write_block)(uint64_t, const uint8_t *, struct _blockdev *);

    uint32_t block_size;
    size_t num_blocks;

    void *_data; // private data for driver
    uint32_t references;
} blockdev_t;

blockdev_t *blockdev_new_ref(blockdev_t *bdev);
int blockdev_free_ref(blockdev_t *bdev);

int blockdev_read_block(uint64_t lba, uint8_t *data, blockdev_t *bdev);
int blockdev_write_block(uint64_t lba, const uint8_t *data, blockdev_t *bdev);

#endif
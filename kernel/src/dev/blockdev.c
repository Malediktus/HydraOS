#include <kernel/dev/blockdev.h>
#include <stddef.h>

blockdev_t *blockdev_new_ref(blockdev_t *bdev)
{
    if (!bdev)
    {
        return NULL;
    }

    bdev->references++;
    return bdev;
}

int blockdev_free_ref(blockdev_t *bdev)
{
    if (!bdev || !bdev->free)
    {
        return -1;
    }

    if (bdev->references <= 1)
    {
        return bdev->free(bdev);
    }
    bdev->references--;

    return 0;
}

int blockdev_read_block(uint64_t lba, uint8_t *data, blockdev_t *bdev)
{
    if (!bdev || !bdev->read_block)
    {
        return -1;
    }

    return bdev->read_block(lba, data, bdev);
}

int blockdev_write_block(uint64_t lba, const uint8_t *data, blockdev_t *bdev)
{
    if (!bdev || !bdev->write_block)
    {
        return -1;
    }

    return bdev->write_block(lba, data, bdev);
}

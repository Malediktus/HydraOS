#include <kernel/fs/vpt.h>
#include <kernel/kmm.h>

typedef struct
{
    uint8_t bootable;

    uint8_t start_head;
    uint8_t start_sector : 6;
    uint16_t start_cylinder : 10;

    uint8_t parititon_type;

    uint8_t end_head;
    uint8_t end_sector : 6;
    uint16_t end_cylinder : 10;

    uint32_t start_lba;
    uint32_t length;
} __attribute__((packed)) partition_table_entry_t;

typedef struct
{
    uint8_t bootloader[440];
    uint32_t signature;
    uint16_t unused;
    partition_table_entry_t primary_partitions[4];
    uint16_t magic_number;
} __attribute__((packed)) master_boot_record_t;

void *mbr_init(blockdev_t *bdev)
{
    if (!bdev)
    {
        return NULL;
    }

    master_boot_record_t *mbr = kmalloc(sizeof(master_boot_record_t));
    if (!mbr)
    {
        return NULL;
    }

    if (blockdev_read_block(0, (uint8_t *)mbr, bdev) < 0)
    {
        return NULL;
    }

    return (void *)mbr;
}

int mbr_free(blockdev_t *bdev, void *mbr_data)
{
    if (!bdev || !mbr_data)
    {
        return -EINVARG;
    }

    kfree(mbr_data);

    return 0;
}

int mbr_test(blockdev_t *bdev)
{
    master_boot_record_t mbr;
    if (blockdev_read_block(0, (uint8_t *)&mbr, bdev) < 0)
    {
        return -ETEST;
    }

    if (mbr.magic_number != 0xAA55)
    {
        return -ETEST;
    }

    return 0;
}

int mbr_get(uint8_t index, void *mbr_data, virtual_blockdev_t *vbdev)
{
    if (!mbr_data || !vbdev)
    {
        return -EINVARG;
    }

    master_boot_record_t *mbr = (master_boot_record_t *)mbr_data;
    if (index >= 4 || mbr->primary_partitions[index].parititon_type == 0x00)
    {
        return -ERECOV;
    }

    vbdev->lba_offset = (size_t)mbr->primary_partitions[index].start_lba;
    vbdev->type = (uint32_t)(size_t)mbr->primary_partitions[index].parititon_type;
    vbdev->index = index;

    return 0;
}

partition_table_t mbr_partition_table = {
    .pt_init = &mbr_init,
    .pt_free = &mbr_free,
    .pt_test = &mbr_test,
    .pt_get = &mbr_get
};

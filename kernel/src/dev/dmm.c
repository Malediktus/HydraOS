#include <kernel/dev/dmm.h>
#include <kernel/dev/pci.h>
#include <kernel/kmm.h>

chardev_t *e9_create(void);
chardev_t *vga_create(void);

blockdev_t *ide_create(size_t index);

#define CHARDEV_CAPACITY_INCREASE 3

chardev_t **chardevs = NULL;
size_t chardevs_capacity = 0;
size_t chardevs_size = 0;

#define BLOCKDEV_CAPACITY_INCREASE 3

blockdev_t **blockdevs = NULL;
size_t blockdevs_capacity = 0;
size_t blockdevs_size = 0;

static int init_char_devices(void)
{
    chardevs = kmalloc(sizeof(chardev_t *) * CHARDEV_CAPACITY_INCREASE);
    if (!chardevs)
    {
        return -1;
    }

    chardevs_capacity = CHARDEV_CAPACITY_INCREASE;
    chardevs_size = 0;

    chardevs[chardevs_size] = e9_create();
    if (!chardevs[chardevs_size])
    {
        return -1;
    }
    chardevs_size++;

    return 0;
}

static int init_block_devices(void)
{
    blockdevs = kmalloc(sizeof(blockdev_t *) * BLOCKDEV_CAPACITY_INCREASE);
    if (!blockdevs)
    {
        return -1;
    }

    blockdevs_capacity = BLOCKDEV_CAPACITY_INCREASE;
    blockdevs_size = 0;

    return 0;
}

static int try_init_char_device(pci_device_t *pci_dev)
{
    if (pci_dev->class_code != PCI_CLASS_DISPLAY_CONTROLLER)
    {
        return -1;
    }

    if (pci_dev->subclass_code == PCI_SUBCLASS_VGA_COMP_CONTROLLER)
    {
        if (chardevs_size >= chardevs_capacity)
        {
            chardevs = krealloc(chardevs, chardevs_capacity, chardevs_capacity + CHARDEV_CAPACITY_INCREASE);
            chardevs_capacity += CHARDEV_CAPACITY_INCREASE;
        }

        chardevs[chardevs_size] = vga_create();
        if (!chardevs[chardevs_size])
        {
            return -1;
        }
        chardevs_size++;
        return 0;
    }

    return 0;
}

static int try_init_block_device(pci_device_t *pci_dev)
{
    if (pci_dev->class_code != PCI_CLASS_MASS_STORAGE_CONTROLLER)
    {
        return -1;
    }

    if (pci_dev->subclass_code == PCI_SUBCLASS_IDE_CONTROLLER)
    {
        if (blockdevs_size >= blockdevs_capacity)
        {
            blockdevs = krealloc(blockdevs, blockdevs_capacity, blockdevs_capacity + BLOCKDEV_CAPACITY_INCREASE);
            blockdevs_capacity += BLOCKDEV_CAPACITY_INCREASE;
        }

        for (size_t i = 0; i < 4; i++)
        {
            blockdevs[blockdevs_size] = ide_create(i);
            if (!blockdevs[blockdevs_size])
            {
                continue;
            }
            blockdevs_size++;
        }
        return 0;
    }

    return 0;
}

int init_devices(void)
{
    if (init_char_devices() < 0)
    {
        return -1;
    }

    if (init_block_devices() < 0)
    {
        return -1;
    }

    for (size_t i = 0; i < MAX_PCI_DEVICES; i++)
    {
        pci_device_t *pci_dev = pci_get_device(i);
        if (!pci_dev)
        {
            break;
        }

        if (pci_dev->class_code == PCI_CLASS_DISPLAY_CONTROLLER)
        {
            if (try_init_char_device(pci_dev) < 0)
            {
                return -1;
            }
        }
        else if (pci_dev->class_code == PCI_CLASS_MASS_STORAGE_CONTROLLER)
        {
            if (try_init_block_device(pci_dev) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

void free_devices(void)
{
    for (size_t i = 0; i < chardevs_size; i++)
    {
        chardev_free_ref(chardevs[i]);
    }

    kfree(chardevs);

    for (size_t i = 0; i < blockdevs_size; i++)
    {
        blockdev_free_ref(blockdevs[i]);
    }

    kfree(blockdevs);
}

chardev_t *get_chardev(size_t index)
{
    if (index >= chardevs_size)
    {
        return NULL;
    }

    return chardev_new_ref(chardevs[index]);
}

blockdev_t *get_blockdev(size_t index)
{
    if (index >= blockdevs_size)
    {
        return NULL;
    }

    return blockdev_new_ref(blockdevs[index]);
}

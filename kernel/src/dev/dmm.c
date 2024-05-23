#include <kernel/dev/dmm.h>
#include <kernel/kmm.h>

#define CHARDEV_CAPACITY_INCREASE 3

chardev_t **chardevs = NULL;
size_t chardevs_capacity = 0;
size_t chardevs_size = 0;

chardev_t *e9_create(void);
chardev_t *vga_create(void);

int init_devices(void)
{
    // TODO: use pci and dont hardcode this
    chardevs = kmalloc(sizeof(chardev_t *) * CHARDEV_CAPACITY_INCREASE);
    if (!chardevs)
    {
        return -1;
    }

    chardevs_capacity = CHARDEV_CAPACITY_INCREASE;
    chardevs_size = 0;

    chardevs[0] = e9_create();
    if (!chardevs[0])
    {
        return -1;
    }
    chardevs_size++;

    chardevs[1] = vga_create();
    if (!chardevs[1])
    {
        return -1;
    }
    chardevs_size++;

    return 0;
}

chardev_t *get_chardev(size_t index)
{
    if (index >= chardevs_size)
    {
        return NULL;
    }

    return chardev_new_ref(chardevs[index]);
}

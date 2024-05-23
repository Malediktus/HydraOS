#include <kernel/dev/dmm.h>

#define MAX_CHARDEVS 2

chardev_t chardevs[MAX_CHARDEVS] = {};
size_t num_chardevs = 0;

int e9_create(chardev_t *cdev);
int vga_create(chardev_t *cdev);

int init_devices(void)
{
    // TODO: use pci and dont hardcode this
    if (e9_create(&chardevs[0]) < 0)
    {
        return -1;
    }

    if (vga_create(&chardevs[1]) < 0)
    {
        return -1;
    }

    num_chardevs = 2;
    return 0;
}

chardev_t *get_chardev(size_t index)
{
    if (index >= num_chardevs)
    {
        return NULL;
    }

    return chardev_new_ref(&chardevs[index]);
}

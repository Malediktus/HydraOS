#include <kernel/dev/chardev.h>
#include <kernel/port.h>
#include <stdbool.h>

static bool e9_initialized = false;

int e9_write(char c, chardev_color_t fg, chardev_color_t bg, chardev_t *cdev)
{
    if (!cdev)
    {
        return -1;
    }

    (void)fg;
    (void)bg;

    port_byte_out(0xE9, c);

    return 0;
}

int e9_free(chardev_t *cdev)
{
    if (!cdev)
    {
        return -1;
    }

    return 0;
}

int e9_create(chardev_t *cdev)
{
    if (!cdev)
    {
        return -1;
    }
    if (e9_initialized)
    {
        return -2;
    }
    e9_initialized = true;

    cdev->write = &e9_write;
    cdev->free = &e9_free;
    cdev->references = 1;

    return 0;
}

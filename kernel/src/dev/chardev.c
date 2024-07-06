#include <kernel/dev/chardev.h>
#include <stddef.h>

chardev_t *chardev_new_ref(chardev_t *cdev)
{
    if (!cdev)
    {
        return NULL;
    }

    cdev->references++;
    return cdev;
}

int chardev_free_ref(chardev_t *cdev)
{
    if (!cdev || !cdev->free)
    {
        return -EINVARG;
    }

    if (cdev->references <= 1)
    {
        return cdev->free(cdev);
    }
    cdev->references--;

    return 0;
}

int chardev_write(char c, chardev_color_t fg, chardev_color_t bg, chardev_t *cdev)
{
    if (!cdev || !cdev->write)
    {
        return -EINVARG;
    }

    return cdev->write(c, fg, bg, cdev);
}

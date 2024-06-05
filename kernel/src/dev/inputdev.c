#include <kernel/dev/inputdev.h>
#include <stddef.h>

inputdev_t *inputdev_new_ref(inputdev_t *idev)
{
    if (!idev)
    {
        return NULL;
    }

    idev->references++;
    return idev;
}

int inputdev_free_ref(inputdev_t *idev)
{
    if (!idev || !idev->free)
    {
        return -1;
    }

    if (idev->references <= 1)
    {
        return idev->free(idev);
    }
    idev->references--;

    return 0;
}

int inputdev_poll(inputpacket_t *packet, inputdev_t *idev)
{
    if (!idev || !idev->poll)
    {
        return -1;
    }

    return idev->poll(packet, idev);
}
 
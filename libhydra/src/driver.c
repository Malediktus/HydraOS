#include <hydra/driver.h>
#include <hydra/kernel.h>

int64_t _raw_driver_read(int type, int64_t id, char *data, size_t size)
{
    return (int64_t)syscall(_SYSCALL_DREAD, (uint64_t)type, (uint64_t)id, (uint64_t)data, (uint64_t)size, 0, 0);
}

int64_t _raw_driver_write(int type, int64_t id, const char *data, size_t size)
{
    return (int64_t)syscall(_SYSCALL_DWRITE, (uint64_t)type, (uint64_t)id, (uint64_t)data, (uint64_t)size, 0, 0);
}

int chardev_putc(int64_t id, char c, chardev_color_t fg, chardev_color_t bg)
{
    uint8_t data[2];
    data[0] = c;
    data[1] = (uint8_t)fg | ((uint8_t)bg << 4);

    return (int)_raw_driver_write(DRIVER_TYPE_CHARDEV, id, (char *)data, 1);
}

int chardev_puts(int64_t id, const char *s, chardev_color_t fg, chardev_color_t bg)
{
    while (*s)
    {
        uint8_t data[2];
        data[0] = *s;
        data[1] = (uint8_t)fg | ((uint8_t)bg << 4);

        int res = (int)_raw_driver_write(DRIVER_TYPE_CHARDEV, id, (char *)data, 1);
        if (res < 0)
        {
            return res;
        }

        s++;
    }

    return 0;
}

#ifndef _DRIVER_H
#define _DRIVER_H 1

#include <stdint.h>
#include <stddef.h>

#define DRIVER_TYPE_CHARDEV 0
#define DRIVER_TYPE_INPUTDEV 1

int64_t _raw_driver_read(int type, int64_t id, char *data, size_t size);
int64_t _raw_driver_write(int type, int64_t id, const char *data, size_t size);

typedef enum
{
    CHARDEV_COLOR_BLACK = 0,
    CHARDEV_COLOR_BLUE = 1,
    CHARDEV_COLOR_GREEN = 2,
    CHARDEV_COLOR_CYAN = 3,
    CHARDEV_COLOR_RED = 4,
    CHARDEV_COLOR_MAGENTA = 5,
    CHARDEV_COLOR_BROWN = 6,
    CHARDEV_COLOR_LIGHT_GRAY = 7,
    CHARDEV_COLOR_DARK_GRAY = 8,
    CHARDEV_COLOR_LIGHT_BLUE = 9,
    CHARDEV_COLOR_LIGHT_GREEN = 10,
    CHARDEV_COLOR_LIGHT_CYAN = 11,
    CHARDEV_COLOR_LIGHT_RED = 12,
    CHARDEV_COLOR_PINK = 13,
    CHARDEV_COLOR_YELLOW = 14,
    CHARDEV_COLOR_WHITE = 15,
} chardev_color_t;

int chardev_putc(int64_t id, char c, chardev_color_t fg, chardev_color_t bg);
int chardev_puts(int64_t id, const char *s, chardev_color_t fg, chardev_color_t bg);

#endif
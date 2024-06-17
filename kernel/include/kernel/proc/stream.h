#ifndef _KERNEL_STREAM_H
#define _KERNEL_STREAM_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/dev/devm.h>

typedef enum
{
    STREAM_TYPE_DRIVER = 0,
    // STREAM_TYPE_FILE = 1,
    // STREAM_TYPE_BIDIRECTIONAL = 2
} stream_type_t;

typedef uint8_t (*stream_driver_read)(device_handle_t);
typedef void (*stream_driver_write)(uint8_t, device_handle_t);

typedef struct
{
    stream_type_t type;

    union
    {
        struct
        {
            stream_driver_read read_func;
            stream_driver_write write_func;
            device_handle_t device_handle;
        };
    };
} stream_t;

stream_t *stream_create_driver(device_handle_t device, stream_driver_read read_func, stream_driver_write write_func);
void stream_free(stream_t *stream);

int stream_read(stream_t *stream, uint8_t *buf, size_t size);
int stream_write(stream_t *stream, const uint8_t *buf, size_t size);

#endif
#include <kernel/proc/stream.h>
#include <kernel/kmm.h>

stream_t *stream_create_driver(device_handle_t device, stream_driver_read read_func, stream_driver_write write_func)
{
    stream_t *stream = kmalloc(sizeof(stream_t));
    if (!stream)
    {
        return NULL;
    }

    stream->read_func = read_func;
    stream->write_func = write_func;
    stream->device_handle = device;

    stream->type = STREAM_TYPE_DRIVER;

    return stream;
}

void stream_free(stream_t *stream)
{
    if (stream->type == STREAM_TYPE_DRIVER)
    {
        kfree(stream);
        return;
    }
}

static int _stream_driver_read(stream_t *stream, uint8_t *buf, size_t size)
{
    if (!stream->read_func)
    {
        return -1;
    }

    for (size_t i = 0; i < size; i++)
    {
        buf[i] = stream->read_func(stream->device_handle);
    }

    return 0;
}

int _stream_driver_write(stream_t *stream, const uint8_t *buf, size_t size)
{
    if (!stream->write_func)
    {
        return -1;
    }

    for (size_t i = 0; i < size; i++)
    {
        stream->write_func(buf[i], stream->device_handle);
    }

    return 0;
}

int stream_read(stream_t *stream, uint8_t *buf, size_t size)
{
    if (stream->type == STREAM_TYPE_DRIVER)
    {
        return _stream_driver_read(stream, buf, size);
    }
    return -1;
}

int stream_write(stream_t *stream, const uint8_t *buf, size_t size)
{
    if (stream->type == STREAM_TYPE_DRIVER)
    {
        return _stream_driver_write(stream, buf, size);
    }
    return -1;
}

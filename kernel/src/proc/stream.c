#include <kernel/proc/stream.h>
#include <kernel/kmm.h>

int stream_create_bidirectional(stream_t *stream, uint8_t flags, size_t size)
{
    stream->type = STREAM_TYPE_BIDIRECTIONAL;
    stream->flags = flags;

    stream->buffer = kmalloc(size);
    if (!stream->buffer)
    {
        return -ENOMEM;
    }

    stream->size = 0;
    stream->max_size = size;

    return 0;
}

int stream_create_file(stream_t *stream, uint8_t flags, const char *path, uint8_t open_action)
{
    stream->type = STREAM_TYPE_FILE;
    stream->flags = flags;

    stream->node = vfs_open(path, open_action);
    if (!stream->node)
    {
        return -ERECOV;
    }

    return 0;
}

int stream_create_driver(stream_t *stream, uint8_t flags, device_handle_t device)
{
    stream->type = STREAM_TYPE_DRIVER;
    stream->flags = flags;

    stream->device = device;

    return 0;
}

void stream_free(stream_t *stream)
{
    switch (stream->type)
    {
    case STREAM_TYPE_BIDIRECTIONAL:
        kfree(stream->buffer);
        break;
    case STREAM_TYPE_FILE:
        vfs_close(stream->node);
        break;
    case STREAM_TYPE_DRIVER:
        break;
    default:
        // TODO: PANIC
        break;
    }
}

int stream_read(stream_t *stream, uint8_t *data, size_t size, size_t *bytes_read)
{
    if (!stream || !data || !bytes_read)
    {
        return -EINVARG;
    }

    *bytes_read = 0;
    switch (stream->type)
    {
    case STREAM_TYPE_BIDIRECTIONAL:
        for (size_t i = 0; i < size && stream->size > 0; i++)
        {
            data[i] = stream->buffer[i];            
            (*bytes_read)++;
            stream->size--;
        }
        break;
    case STREAM_TYPE_FILE:
        // TODO: maybe check for filesize and offset
        *bytes_read = size;
        int res = vfs_read(stream->node, size, data);
        if (res < 0)
        {
            return -ERECOV;
        }

        break;
    case STREAM_TYPE_DRIVER:
        switch (stream->device.type)
        {
        case DEVICE_TYPE_INPUTDEV:
            inputpacket_t packet;
            size_t i = 0;
            while (inputdev_poll(&packet, stream->device.idev) == 0 && packet.type != IPACKET_NULL && i < size)
            {
                if (packet.type != IPACKET_KEYDOWN && packet.type != IPACKET_KEYREPEAT)
                {
                    continue;
                }
                data[i++] = inputdev_packet_to_ascii(&packet);
                (*bytes_read)++;
            }

            break;
        case DEVICE_TYPE_BLOCKDEV:
            // TODO: implement
            break;
        case DEVICE_TYPE_CHARDEV:
            return -ERECOV;
        default:
            return -EINVARG;
        }
        break;
    default:
        return -EINVARG;
    }

    return 0;
}

int stream_write(stream_t *stream, const uint8_t *data, size_t size, size_t *bytes_written)
{
    if (!stream || !data || !bytes_written)
    {
        return -EINVARG;
    }

    *bytes_written = 0;
    switch (stream->type)
    {
    case STREAM_TYPE_BIDIRECTIONAL:
        for (size_t i = 0; i < size && stream->size < stream->max_size; i++)
        {
            stream->buffer[stream->size++] = data[i];            
            (*bytes_written)++;
        }
        break;
    case STREAM_TYPE_FILE:
        *bytes_written = size;
        int res = vfs_write(stream->node, size, data);
        if (res < 0)
        {
            return -ERECOV;
        }

        break;
    case STREAM_TYPE_DRIVER:
        switch (stream->device.type)
        {
        case DEVICE_TYPE_CHARDEV:
            for (size_t i = 0; i < size; i++)
            {
                int res = chardev_write((char)data[i], CHARDEV_COLOR_WHITE, CHARDEV_COLOR_BLACK, stream->device.cdev);
                if (res < 0)
                {
                    return res;
                }

                (*bytes_written)++;
            }

            break;
        case DEVICE_TYPE_BLOCKDEV:
            // TODO: implement
            break;

        case DEVICE_TYPE_INPUTDEV:
            return -ERECOV;
        default:
            return -EINVARG;
        }
        break;
    default:
        return -EINVARG;
    }

    return 0;
}

int stream_flush(stream_t *stream)
{
    if (stream->type == STREAM_TYPE_BIDIRECTIONAL)
    {
        stream->size = 0;
    }

    return 0;
}

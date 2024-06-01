#include <kernel/fs/vfs.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

#define FILESYSTEMS_CAPACITY_INCREASE 3

filesystem_t **filesystems = NULL;
size_t filesystems_capacity = 0;
size_t filesystems_size = 0;

int register_filesystem(filesystem_t *fs)
{
    if (!fs)
    {
        return -1;
    }

    if (!filesystems)
    {
        filesystems = kmalloc(sizeof(filesystem_t *) * FILESYSTEMS_CAPACITY_INCREASE);
        if (!filesystems)
        {
            return -1;
        }

        filesystems_capacity = FILESYSTEMS_CAPACITY_INCREASE;
        filesystems_size = 0;
    }

    if (filesystems_size >= filesystems_capacity)
    {
        filesystems = krealloc(filesystems, filesystems_capacity, filesystems_capacity + sizeof(filesystem_t *) * FILESYSTEMS_CAPACITY_INCREASE);
        filesystems_capacity += FILESYSTEMS_CAPACITY_INCREASE;
    }

    filesystems[filesystems_size] = fs;

    return 0;
}

typedef struct _mount
{
    blockdev_t *bdev;
    filesystem_t *fs;
    int id;

    struct _mount *prev;
    struct _mount *next;
    void *fs_data;
} mount_t;

mount_t *mounts_head;

static int allocate_mount_id(void)
{
    for (int i = 0; i < INT32_MAX; i++)
    {
        bool found = false;
        for (mount_t *mnt = mounts_head; mnt != NULL; mnt = mnt->next)
        {
            if (mnt->id == i)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return i;
        }
    }

    return -1;
}

int vfs_mount_blockdev(blockdev_t *bdev)
{
    if (!bdev)
    {
        return -1;
    }

    filesystem_t *fs = NULL;
    for (size_t i = 0; i < filesystems_size; i++)
    {
        if (filesystems[i]->fs_test(bdev) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    if (!fs)
    {
        return -1;
    }

    if (!mounts_head)
    {
        mounts_head = kmalloc(sizeof(mount_t));
        if (!mounts_head)
        {
            return -1;
        }

        mounts_head->bdev = blockdev_new_ref(bdev);
        mounts_head->fs = fs;
        mounts_head->fs_data = fs->fs_init(bdev);
        mounts_head->id = allocate_mount_id();
        if (mounts_head->id < 0)
        {
            return -1;
        }

        mounts_head->next = NULL;
        mounts_head->prev = NULL;
        return mounts_head->id;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt->next != NULL; mnt = mnt->next)
        ;

    if (!mnt)
    {
        return -1;
    }

    mount_t *new_mount = kmalloc(sizeof(mount_t));
    if (!new_mount)
    {
        return -1;
    }
    mnt->next = new_mount;

    new_mount->bdev = blockdev_new_ref(bdev);
    new_mount->fs = fs;
    new_mount->fs_data = fs->fs_init(bdev);
    new_mount->id = allocate_mount_id();
    if (new_mount->id < 0)
    {
        return -1;
    }

    new_mount->next = NULL;
    new_mount->prev = mnt;
    return new_mount->id;
}

int vfs_unmount_blockdev(int id)
{
    if (!mounts_head)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != id)
        {
            continue;
        }

        mnt->prev->next = mnt->next;
        mnt->next->prev = mnt->prev;

        mnt->fs->fs_free(mnt->bdev, mnt->fs_data);

        if (blockdev_free_ref(mnt->bdev) < 0)
        {
            return -1;
        }

        kfree(mnt);
        return 0;
    }

    return -1;
}

int extract_disk_id(const char *str)
{
    if (!str)
    {
        return -1;
    }

    char *end;
    long id = strtol(str, &end, 10);

    if (end == str || *end != ':')
    {
        return -1;
    }

    if (id < 0 || id > INT32_MAX)
    {
        return -1;
    }

    return (int)id;
}

const char *extract_path(const char *str)
{
    if (!str)
    {
        return NULL;
    }

    const char *colon_pos = strchr(str, ':');
    if (!colon_pos || *(colon_pos + 1) != '/')
    {
        return NULL;
    }

    return (const char *)((uintptr_t)colon_pos + 1);
}

file_node_t *vfs_open(const char *path, uint8_t action)
{
    if (!path)
    {
        return NULL;
    }

    int id = extract_disk_id(path);
    if (id < 0)
    {
        return NULL;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != id)
        {
            continue;
        }

        const char *local_path = extract_path(path);
        if (!local_path)
        {
            return NULL;
        }

        return mnt->fs->fs_open(local_path, action, mnt->bdev, mnt->fs_data);
    }

    return NULL;
}

int vfs_close(file_node_t *node)
{
    if (!node)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != node->mount_id)
        {
            continue;
        }

        return mnt->fs->fs_close(node, mnt->bdev, mnt->fs_data);
    }

    return -1;
}

int vfs_read(file_node_t *node, size_t size, uint8_t *buf)
{
    if (!node)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != node->mount_id)
        {
            continue;
        }

        return mnt->fs->fs_read(node, size, buf, mnt->bdev, mnt->fs_data);
    }

    return -1;
}

int vfs_write(file_node_t *node, size_t size, const uint8_t *buf)
{
    if (!node)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != node->mount_id)
        {
            continue;
        }

        return mnt->fs->fs_write(node, size, buf, mnt->bdev, mnt->fs_data);
    }

    return -1;
}

int vfs_readdir(file_node_t *node, dirent_t *dirent)
{
    if (!node)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != node->mount_id)
        {
            continue;
        }

        return mnt->fs->fs_readdir(node, dirent, mnt->bdev, mnt->fs_data);
    }

    return -1;
}

int vfs_delete(file_node_t *node)
{
    if (!node)
    {
        return -1;
    }

    mount_t *mnt = NULL;
    for (mnt = mounts_head; mnt != NULL; mnt = mnt->next)
    {
        if (mnt->id != node->mount_id)
        {
            continue;
        }

        return mnt->fs->fs_delete(node, mnt->bdev, mnt->fs_data);
    }

    return -1;
}

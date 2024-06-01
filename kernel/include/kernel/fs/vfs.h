#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/dev/blockdev.h>

#define MAX_PATH 256

typedef struct
{
    char path[MAX_PATH];
} directory_entry_t;

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_SYMLINK 0x03

#define MASK_READONLY 1 << 1
#define MASK_HIDDEN 1 << 2
#define MASK_SYSTEM 1 << 3

typedef struct
{
    char local_path[MAX_PATH];
    int mount_id;

    size_t offset;

    size_t filesize;
    uint32_t mask;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t last_access_date;
    uint32_t flags;

    uint64_t _data; // private data for fs implementation
} file_node_t;

typedef struct
{
    const char path[MAX_PATH + 3]; // global path
} dirent_t;

int vfs_mount_blockdev(blockdev_t *bdev);
int vfs_unmount_blockdev(int id);

#define OPEN_ACTION_WRITE 1 << 1
#define OPEN_ACTION_CLEAR 1 << 2
#define OPEN_ACTION_CREATE 1 << 3

file_node_t *vfs_open(const char *path, uint8_t action);
int vfs_close(file_node_t *node);
int vfs_read(file_node_t *node, size_t size, uint8_t *buf);
int vfs_write(file_node_t *node, size_t size, const uint8_t *buf);
int vfs_readdir(file_node_t *node, dirent_t *dirent);
int vfs_delete(file_node_t *node); // doesnt close node

typedef struct
{
    void *(*fs_init)(blockdev_t *);
    int (*fs_free)(blockdev_t *, void *);
    int (*fs_test)(blockdev_t *);

    file_node_t *(*fs_open)(const char *, uint8_t, blockdev_t *, void *);
    int (*fs_close)(file_node_t *, blockdev_t *, void *);
    int (*fs_read)(file_node_t *, size_t, uint8_t *, blockdev_t *, void *);
    int (*fs_write)(file_node_t *, size_t, const uint8_t *, blockdev_t *, void *);
    int (*fs_readdir)(file_node_t *, dirent_t *, blockdev_t *, void *);
    int (*fs_delete)(file_node_t *, blockdev_t *, void *);
} filesystem_t;

int register_filesystem(filesystem_t *fs);

#endif
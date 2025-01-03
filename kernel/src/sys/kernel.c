#include <kernel/multiboot2.h>
#include <kernel/string.h>
#include <kernel/dev/pci.h>
#include <kernel/dev/devm.h>
#include <kernel/kprintf.h>
#include <kernel/smm.h>
#include <kernel/isr.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/kmm.h>
#include <kernel/fs/vpt.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/task.h>
#include <kernel/proc/scheduler.h>
#include <kernel/pit.h>

extern partition_table_t mbr_partition_table;
extern filesystem_t fat32_filesystem;

typedef struct
{
    uint8_t tty;
    uint64_t total_memory;
    uint64_t num_mmap_entries;
    memory_map_entry_t memory_map[20];
} boot_info_t;

static int process_boot_parameter(const char *key, char *value, boot_info_t *boot_info)
{
    if (strcmp(key, "klog") == 0)
    {
        if (strlen(value) < 4)
        {
            return -1;
        }
        boot_info->tty = atoui(value + 3);
    }
    return 0;
}

static int process_boot_parameters(char *parameters, boot_info_t *boot_info)
{
    char *key_start = parameters;
    char *value_start = NULL;

    while (*parameters != '\0')
    {
        if (*parameters == '=')
        {
            *parameters = '\0';
            value_start = parameters + 1;
        }
        else if (*parameters == ' ' || *(parameters + 1) == '\0')
        {
            if (*parameters == ' ')
            {
                *parameters = '\0';
            }
            else if (*(parameters + 1) == '\0')
            {
                parameters++;
            }
            if (value_start)
            {
                if (process_boot_parameter(key_start, value_start, boot_info) < 0)
                {
                    return -1;
                }
            }
            key_start = parameters + 1;
            value_start = NULL;
        }
        parameters++;
    }

    return 0;
}

static int process_mmap(struct multiboot_tag_mmap *multiboot2_mmap, boot_info_t *boot_info)
{
    for (multiboot_memory_map_t *entry = multiboot2_mmap->entries;
        (uintptr_t)entry < (uintptr_t)multiboot2_mmap + multiboot2_mmap->size;
        entry = (multiboot_memory_map_t *)((uintptr_t)entry + multiboot2_mmap->entry_size))
    {
        switch (entry->type)
        {
        case MULTIBOOT_MEMORY_AVAILABLE:
            boot_info->total_memory = entry->addr + entry->len;

            boot_info->memory_map[boot_info->num_mmap_entries].addr = entry->addr;
            boot_info->memory_map[boot_info->num_mmap_entries].size = entry->len;
            boot_info->memory_map[boot_info->num_mmap_entries].type = MMAP_ENTRY_TYPE_AVAILABLE;
            if (entry->addr == 0)
            {
                boot_info->memory_map[boot_info->num_mmap_entries].addr += 4096;
                boot_info->memory_map[boot_info->num_mmap_entries].size -= 4096;
            }
            break;
        case MULTIBOOT_MEMORY_RESERVED:
            boot_info->memory_map[boot_info->num_mmap_entries].addr = entry->addr;
            boot_info->memory_map[boot_info->num_mmap_entries].size = entry->len;
            boot_info->memory_map[boot_info->num_mmap_entries].type = MMAP_ENTRY_TYPE_RESERVED;
            break;
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            boot_info->memory_map[boot_info->num_mmap_entries].addr = entry->addr;
            boot_info->memory_map[boot_info->num_mmap_entries].size = entry->len;
            boot_info->memory_map[boot_info->num_mmap_entries].type = MMAP_ENTRY_TYPE_ACPI_RECLAIMABLE;
            break;
        case MULTIBOOT_MEMORY_NVS:
            boot_info->memory_map[boot_info->num_mmap_entries].addr = entry->addr;
            boot_info->memory_map[boot_info->num_mmap_entries].size = entry->len;
            boot_info->memory_map[boot_info->num_mmap_entries].type = MMAP_ENTRY_TYPE_RESERVED;
            break;
        case MULTIBOOT_MEMORY_BADRAM:
            boot_info->memory_map[boot_info->num_mmap_entries].addr = entry->addr;
            boot_info->memory_map[boot_info->num_mmap_entries].size = entry->len;
            boot_info->memory_map[boot_info->num_mmap_entries].type = MMAP_ENTRY_TYPE_RESERVED;
            break;
        }
        kprintf("MEMMAP ENTRY: addr 0x%p, size 0x%x, type %d\n", (void*)boot_info->memory_map[boot_info->num_mmap_entries].addr, boot_info->memory_map[boot_info->num_mmap_entries].size, boot_info->memory_map[boot_info->num_mmap_entries].type);

        boot_info->num_mmap_entries++;
    }

    return 0;
}

static int parse_multiboot2_structure(uint64_t multiboot2_struct_addr, boot_info_t *boot_info)
{
    if (multiboot2_struct_addr & 7)
    {
        return -1;
    }

    boot_info->tty = 0;
    boot_info->total_memory = 0;
    boot_info->num_mmap_entries = 0;
    memset(boot_info->memory_map, 0, sizeof(boot_info->memory_map));

    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *)(multiboot2_struct_addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        if (tag->type == MULTIBOOT_TAG_TYPE_CMDLINE)
        {
            if (process_boot_parameters(((struct multiboot_tag_string *)tag)->string, boot_info) < 0)
            {
                return -1;
            }
        }
        else if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            if (process_mmap((struct multiboot_tag_mmap *)tag, boot_info) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

page_table_t *kernel_pml4 = NULL;

void kmain(uint64_t multiboot2_struct_addr)
{
    boot_info_t boot_info = {0};
    if (parse_multiboot2_structure(multiboot2_struct_addr, &boot_info) < 0)
    {
        return;
    }

    if (segmentation_init() < 0)
    {
        return;
    }

    if (pmm_init(boot_info.memory_map, boot_info.num_mmap_entries, boot_info.total_memory) < 0) // TODO: add 64 bit address range
    {
        return;
    }
    
    kernel_pml4 = pmm_alloc();
    if (!kernel_pml4)
    {
        return;
    }
    
    uint64_t total_pages = boot_info.total_memory / PAGE_SIZE;
    for (uint64_t page = 0; page < total_pages; page++)
    {
        if (pml4_map(kernel_pml4, (void *)(page * PAGE_SIZE), (void *)(page * PAGE_SIZE), PAGE_PRESENT | PAGE_WRITABLE) < 0)
        {
            return;
        }
    }

    if (pml4_switch(kernel_pml4) < 0)
    {
        return;
    }

    if (kmm_init(kernel_pml4, 0x1200000, 32 * PAGE_SIZE, 16) < 0) // TODO: make dynamicly grow
    {
        return;
    }

    if (pci_init() < 0)
    {
        return;
    }


    if (interrupts_init() < 0)
    {
        return;
    }

    if (pit_init(100) < 0)
    {
        return;
    }

    enable_interrupts();

    if (init_devices() < 0)
    {
        return;
    }

    if (kprintf_init(get_chardev(boot_info.tty)) < 0)
    {
        return;
    }

    if (register_partition_table(&mbr_partition_table) < 0)
    {
        KPANIC("failed to register mbr partition table");
    }

    if (register_filesystem(&fat32_filesystem) < 0)
    {
        KPANIC("failed to register fat32 filesystem");
    }

    uint64_t i = 0;
    blockdev_t *bdev = NULL;
    do
    {
        i++;
        bdev = get_blockdev(i-1);
        if (!bdev)
        {
            break;
        }
        kprintf("found disc %s with %ld sectors, id %ld\n", bdev->model, bdev->num_blocks, i-1);

        if (scan_partition(bdev) < 0)
        {
            kprintf("\x1b[31mfailed to scan partition table\n");
            continue;
        }

        for (int j = 0; j < 4; j++)
        {
            virtual_blockdev_t *part = get_virtual_blockdev(bdev, j);
            if (!part)
            {
                break;
            }
            kprintf(" - parititon type: 0x%x\n", part->type);
            if (part->type == 0x83)
            {
                if (vfs_mount_blockdev(part) < 0)
                {
                    KPANIC("failed to mount partition");
                }
                kprintf("mounted\n");
            }
        }
    } while (i > 0);

    kprintf("initializing the kernel\n");
    kprintf("\x1b[31mRed\x1b[0m \x1b[32mGreen\x1b[0m \x1b[33mYellow\x1b[0m \x1b[34mBlue\x1b[0m \x1b[35mMagenta\x1b[0m \x1b[36mCyan\x1b[0m\n");

    kprintf("starting user program...\n");
    process_t *proc = process_create("0:/bin/program");
    if (!proc)
    {
        KPANIC("failed to load '0:/bin/program'");
    }
    if (process_register(proc) < 0)
    {
        KPANIC("failed to register process");
    }

    scheduler_init();

    syscall_init();
    execute_next_process();

    KPANIC("failed to launch '0:/bin/program'");
}

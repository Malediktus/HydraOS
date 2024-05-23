#include <kernel/multiboot2.h>
#include <kernel/string.h>
#include <kernel/dev/dmm.h>
#include <kernel/kprintf.h>

typedef struct
{
    uint8_t tty;
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

static int parse_multiboot2_structure(uint64_t multiboot2_struct_addr, boot_info_t *boot_info)
{
    if (multiboot2_struct_addr & 7)
    {
        return -1;
    }

    boot_info->tty = 0;

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
    }

    return 0;
}

void kmain(uint64_t multiboot2_struct_addr)
{
    boot_info_t boot_info = {0};
    if (parse_multiboot2_structure(multiboot2_struct_addr, &boot_info) < 0)
    {
        return;
    }

    if (init_devices() < 0)
    {
        return;
    }

    if (kprintf_init(get_chardev(boot_info.tty)) < 0)
    {
        return;
    }

    kprintf("initializing the kernel\n");
    kprintf("\x1b[31mRed\x1b[0m \x1b[32mGreen\x1b[0m \x1b[33mYellow\x1b[0m \x1b[34mBlue\x1b[0m \x1b[35mMagenta\x1b[0m \x1b[36mCyan\x1b[0m");

    kprintf_free();
    while (1);
}

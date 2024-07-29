#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>
#include <kernel/string.h>
#include <kernel/dev/devm.h>

#define DRIVER_TYPE_CHARDEV 0
#define DRIVER_TYPE_INPUTDEV 1

int64_t syscall_dread(process_t *proc, int64_t type, int64_t id, int64_t data, int64_t size, int64_t, int64_t, task_state_t *)
{
    if (size < 9)
    {
        return -4;
    }

    size_t offset = (uint64_t)data % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((data / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -EUNKNOWN;
    }

    uint8_t *buf = (uint8_t *)(t + offset);

    switch (type)
    {
    case DRIVER_TYPE_INPUTDEV:
        inputdev_t *idev = get_inputdev(id);
        if (!idev)
        {
            return -2;
        }

        inputpacket_t packet;
        int res = inputdev_poll(&packet, idev);
        if (res < 0)
        {
            return -3;
        }

        buf[0] = packet.type;
        switch (packet.type)
        {
        case IPACKET_NULL:
            break;

        case IPACKET_KEYDOWN:
        case IPACKET_KEYUP:
        case IPACKET_KEYREPEAT:
            buf[1] = packet.modifier;
            buf[2] = packet.scancode;
            break;
        
        default:
            return -5;
        }

        break;

    default:
        return -1;
    }

    return 0;
}

int64_t syscall_dwrite(process_t *proc, int64_t type, int64_t id, int64_t data, int64_t size, int64_t, int64_t, task_state_t *)
{
    size_t offset = (uint64_t)data % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((data / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -EUNKNOWN;
    }

    uint8_t *buf = (uint8_t *)(t + offset);

    switch (type)
    {
    case DRIVER_TYPE_CHARDEV:
        chardev_t *cdev = get_chardev(id);
        if (!cdev)
        {
            return -2;
        }

        for (size_t i = 0; i < (size_t)size; i++)
        {
            chardev_color_t fg = (chardev_color_t)(buf[i+1] & 0xF);
            chardev_color_t bg = (chardev_color_t)((buf[i+1] >> 4) & 0xF);

            int res = chardev_write(buf[i], fg, bg, cdev);
            if (res < 0)
            {
                return -3;
            }
        }

        break;

    default:
        return -1;
    }

    return 0;
}

int64_t syscall_fork(process_t *proc, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, task_state_t *state)
{
    process_t *fork = process_create(proc->path); // TODO: maybe the file changed
    if (!fork)
    {
        while (1);
        // TODO: panic
    }

    for (size_t i = 0; i < fork->task->num_stack_pages; i++)
    {
        memcpy(fork->task->stack_pages[i], proc->task->stack_pages[i], PAGE_SIZE);
    }

    memcpy(&fork->task->state, state, sizeof(task_state_t));

    for (uint64_t i = 0; i < PROCESS_MAX_HEAP_PAGES; i++)
    {
        if (proc->allocations[i] == 0)
        {
            continue;
        }

        fork->allocations[i] = pmm_alloc();
        if (!fork->allocations[i])
        {
            // TODO: panic
            while (1);
        }
        memcpy(fork->allocations[i], proc->allocations[i], PAGE_SIZE);

        if (pml4_map(fork->pml4, (void *)(PROCESS_HEAP_VADDR_BASE + i * PAGE_SIZE), fork->allocations[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            // TODO: panic
            while (1);
        }
    }

    fork->task->state.rax = 0; // return value

    if (process_register(fork) < 0)
    {
        while (1);
        // TODO: panic
    }

    return fork->pid;
}

int64_t syscall_alloc(process_t *proc, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, task_state_t *)
{
    return (int64_t)process_allocate_page(proc);
}

int64_t syscall_fopen(process_t *proc, int64_t _path, int64_t action, int64_t, int64_t, int64_t, int64_t, task_state_t *)
{
    size_t offset = (uint64_t)_path % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (char *)((_path / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -EUNKNOWN;
    }

    char *path = (char *)(t + offset);

    return (int64_t)process_open_file(proc, path, (uint8_t)action);
}

int64_t syscall_fclose(process_t *proc, int64_t id, int64_t, int64_t, int64_t, int64_t, int64_t, task_state_t *)
{
    return (int64_t)process_close_file(proc, (uint64_t)id);
}

int64_t syscall_fread(process_t *proc, int64_t id, int64_t _buf, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    size_t offset = (uint64_t)_buf % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((_buf / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -EUNKNOWN;
    }

    void *buf = (void *)(t + offset);

    file_node_t *node = process_get_file(proc, id);
    if (!node)
    {
        return -EINVARG;
    }
    return (int64_t)vfs_read(node, size, buf);
}

int64_t syscall_fwrite(process_t *proc, int64_t id, int64_t _buf, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    size_t offset = (uint64_t)_buf % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((_buf / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -EUNKNOWN;
    }

    void *buf = (void *)(t + offset);

    file_node_t *node = process_get_file(proc, id);
    if (!node)
    {
        return -EINVARG;
    }
    return (int64_t)vfs_write(node, size, buf);
}

extern page_table_t *kernel_pml4;

int64_t syscall_handler(uint64_t num, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5, task_state_t *state)
{
    if (pml4_switch(kernel_pml4) < 0)
    {
        // TODO: panic
        while (1);
    }

    process_t *proc = get_current_process();
    if (!proc)
    {
        // TODO: panic
        while (1);
    }

    int64_t res = -1;
    switch (num)
    {
    case 0:
        res = syscall_dread(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 1:
        res = syscall_dwrite(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 2:
        res = syscall_fork(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 3:
        res = syscall_alloc(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 4:
        res = syscall_fopen(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 5:
        res = syscall_fclose(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 6:
        res = syscall_fread(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 7:
        res = syscall_fwrite(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    default:
        break;
    }

    if (pml4_switch(proc->pml4) < 0)
    {
        // TODO: panic
        while (1);
    }

    return res;
}

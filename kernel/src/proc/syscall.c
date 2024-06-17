#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>

int64_t syscall_read(process_t *proc, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5)
{
    stream_t *stream = NULL;
    if (arg0 == 0)
    {
        stream = proc->stdin;
    }
    else if (arg0 == 1)
    {
        stream = proc->stdout;
    }
    else if (arg0 == 2)
    {
        stream = proc->stderr;
    }
    else
    {
        return -1;
    }

    size_t offset = (uint64_t)arg1 % PAGE_SIZE;
    uint64_t t = (void *)pml4_get_phys(proc->pml4, (void *)((arg1 / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -1;
    }

    void *buf = (void *)(t + offset);

    return stream_read(stream, buf, arg2);
}

int64_t syscall_write(process_t *proc, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5)
{
    stream_t *stream = NULL;
    if (arg0 == 0)
    {
        stream = proc->stdin;
    }
    else if (arg0 == 1)
    {
        stream = proc->stdout;
    }
    else if (arg0 == 2)
    {
        stream = proc->stderr;
    }
    else
    {
        return -1;
    }

    size_t offset = (uint64_t)arg1 % PAGE_SIZE;
    uint64_t t = (void *)pml4_get_phys(proc->pml4, (void *)((arg1 / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -1;
    }

    void *buf = (void *)(t + offset);

    return stream_write(stream, buf, arg2);
}

extern page_table_t *kernel_pml4;

int64_t syscall_handler(uint64_t num, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5)
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
        res = syscall_read(proc, arg0, arg1, arg2, arg3, arg4, arg5);
        break;
    case 1:
        res = syscall_write(proc, arg0, arg1, arg2, arg3, arg4, arg5);
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

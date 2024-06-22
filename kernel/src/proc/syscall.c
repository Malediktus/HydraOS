#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>
#include <kernel/string.h>

int64_t syscall_read(process_t *proc, int64_t _stream, int64_t data, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    stream_t *stream = NULL;
    if (_stream == 0)
    {
        stream = proc->stdin;
    }
    else if (_stream == 1)
    {
        stream = proc->stdout;
    }
    else if (_stream == 2)
    {
        stream = proc->stderr;
    }
    else
    {
        return -1;
    }

    size_t offset = (uint64_t)data % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((data / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -1;
    }

    void *buf = (void *)(t + offset);

    return stream_read(stream, buf, size);
}

int64_t syscall_write(process_t *proc, int64_t _stream, int64_t data, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    stream_t *stream = NULL;
    if (_stream == 0)
    {
        stream = proc->stdin;
    }
    else if (_stream == 1)
    {
        stream = proc->stdout;
    }
    else if (_stream == 2)
    {
        stream = proc->stderr;
    }
    else
    {
        return -1;
    }

    size_t offset = (uint64_t)data % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((data / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return -1;
    }

    void *buf = (void *)(t + offset);

    return stream_write(stream, buf, size);
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

    fork->task->state.rax = 0; // return value
    //fork->task->state.rip++;

    if (process_register(fork) < 0)
    {
        while (1);
        // TODO: panic
    }

    return fork->pid;
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
        res = syscall_read(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 1:
        res = syscall_write(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 2:
        res = syscall_fork(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
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

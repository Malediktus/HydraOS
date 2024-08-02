#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>
#include <kernel/string.h>
#include <kernel/dev/devm.h>

static void *process_get_pointer(process_t *proc, uintptr_t vaddr)
{
    size_t offset = (uint64_t)vaddr % PAGE_SIZE;
    uint64_t t = pml4_get_phys(proc->pml4, (void *)((vaddr / PAGE_SIZE) * PAGE_SIZE), true);
    if (t == 0)
    {
        return NULL;
    }

    return (void *)(t + offset);
}

#define DRIVER_TYPE_CHARDEV 0
#define DRIVER_TYPE_INPUTDEV 1

int64_t syscall_read(process_t *proc, int64_t stream, int64_t data, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    uint8_t *buf = (uint8_t *)process_get_pointer(proc, data);
    if (!buf)
    {
        return -EUNKNOWN;
    }

    size_t bytes_read = 0;
    int res = stream_read(&proc->streams[stream], buf, size, &bytes_read);
    if (res < 0)
    {
        return res;
    }

    return (int64_t)bytes_read;
}

int64_t syscall_write(process_t *proc, int64_t stream, int64_t data, int64_t size, int64_t, int64_t, int64_t, task_state_t *)
{
    uint8_t *buf = (uint8_t *)process_get_pointer(proc, data);
    if (!buf)
    {
        return -EUNKNOWN;
    }

    size_t bytes_written = 0;
    int res = stream_write(&proc->streams[stream], buf, size, &bytes_written);
    if (res < 0)
    {
        return res;
    }

    return (int64_t)bytes_written;
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

    for (uint64_t i = 0; i < PROCESS_MAX_STREAMS; i++)
    {
        if (proc->streams[i].type != STREAM_TYPE_NULL)
        {
            int res = stream_clone(&proc->streams[i], &fork->streams[i]);
            if (res < 0)
            {
                process_free(fork);
                return res;
            }
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

int64_t syscall_exit(process_t *proc, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, task_state_t *)
{
    process_unregister(proc);
    process_free(proc);
    execute_next_process();

    while (1);
    // TODO: panic
}

int64_t syscall_ping(process_t *proc, int64_t pid, int64_t, int64_t, int64_t, int64_t, int64_t, task_state_t *)
{
    if (get_process_from_pid((uint64_t)pid) != NULL)
    {
        return pid;
    }

    return 0;
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
    case 3:
        res = syscall_exit(proc, arg0, arg1, arg2, arg3, arg4, arg5, state);
        break;
    case 4:
        res = syscall_ping(proc, arg0, arg1, arg2, arg3, arg4, arg5, state); // TODO: this is a hack to replace wait because i dont have signals yet
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

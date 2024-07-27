#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>
#include <kernel/kprintf.h>
#include <kernel/pmm.h>

extern int __kernel_start;
extern int __kernel_end;

static uint32_t stdin_read(device_handle_t handle)
{
    inputpacket_t packet;
    if (inputdev_poll(&packet, handle.idev) < 0)
    {
        // TODO: panic
        while (1);
    }

    if (packet.type == IPACKET_NULL || packet.type == IPACKET_KEYUP)
    {
        return 0;
    }

    uint8_t res[2];
    res[0] = packet.scancode;
    res[1] = packet.modifier;
    uint32_t t = 0;
    memcpy(&t, res, 2);
    return t;
}

static void stdout_write(uint32_t data, device_handle_t handle)
{
    if (chardev_write((char)data, CHARDEV_COLOR_WHITE, CHARDEV_COLOR_BLACK, handle.cdev))
    {
        // TODO: panic
        while (1);
    }
}

static void stderr_write(uint32_t data, device_handle_t handle)
{
    if (chardev_write((char)data, CHARDEV_COLOR_RED, CHARDEV_COLOR_BLACK, handle.cdev))
    {
        // TODO: panic
        while (1);
    }
}

static uint64_t current_pid = 0;
process_t *process_create(const char *path)
{
    process_t *proc = kmalloc(sizeof(process_t));
    if (!proc)
    {
        return NULL;
    }

    memset(proc, 0, sizeof(process_t));

    proc->elf = elf_load(path);
    if (!proc->elf)
    {
        process_free(proc);
        return NULL;
    }

    proc->task = kmalloc(sizeof(task_t));
    if (!proc->task)
    {
        process_free(proc);
        return NULL;
    }

    memset(&proc->task->state, 0, sizeof(task_state_t));
    proc->task->state.rip = elf_entry(proc->elf);

    strncpy(proc->path, path, MAX_PATH);
    proc->pml4 = pmm_alloc();
    memset(proc->pml4, 0, PAGE_SIZE);
    if (!proc->pml4)
    {
        process_free(proc);
        return NULL;
    }

    proc->task->parent = proc;

    for (uintptr_t i = (uintptr_t)&__kernel_start; i < (uintptr_t)&__kernel_end; i += PAGE_SIZE)
    {
        if (pml4_map(proc->pml4, (void *)i, (void *)i, PAGE_PRESENT | PAGE_WRITABLE) < 0)
        {
            process_free(proc);
            return NULL;
        }
    }

    proc->task->num_stack_pages = PROCESS_STACK_SIZE / PAGE_SIZE;
    proc->task->stack_pages = kmalloc(proc->task->num_stack_pages);
    if (!proc->task->stack_pages)
    {
        process_free(proc);
        return NULL;
    }

    for (size_t i = 0; i < proc->task->num_stack_pages; i++)
    {
        proc->task->stack_pages[i] = pmm_alloc();
        if (!proc->task->stack_pages[i])
        {
            process_free(proc);
            return NULL;
        }

        if (pml4_map(proc->pml4, (void *)(PROCESS_STACK_VADDR_BASE + (PROCESS_STACK_SIZE - i * PAGE_SIZE)), proc->task->stack_pages[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            process_free(proc);
            return NULL;
        }
    }

    if (elf_load_and_map(proc, proc->elf) < 0)
    {
        process_free(proc);
        return NULL;
    }

    memset(proc->allocations, 0, PROCESS_MAX_HEAP_PAGES * sizeof(void *));
    memset(proc->files, 0, PROCESS_MAX_FILES * sizeof(file_node_t *));

    proc->task->state.rsp = PROCESS_STACK_VADDR_BASE + PROCESS_STACK_SIZE;
    proc->next = NULL;

    device_handle_t stdin_dev;
    stdin_dev.type = DEVICE_TYPE_INPUTDEV;
    stdin_dev.idev = get_inputdev(0);
    if (!stdin_dev.idev)
    {
        process_free(proc);
        return NULL;
    }

    proc->stdin = stream_create_driver(stdin_dev, &stdin_read, NULL);
    if (!proc->stdin)
    {
        process_free(proc);
        return NULL;
    }

    device_handle_t stdout_dev;
    stdout_dev.type = DEVICE_TYPE_CHARDEV;
    stdout_dev.cdev = kprintf_get_cdev();
    if (!stdout_dev.cdev)
    {
        process_free(proc);
        return NULL;
    }

    proc->stdout = stream_create_driver(stdout_dev, NULL, &stdout_write);
    if (!proc->stdout)
    {
        process_free(proc);
        return NULL;
    }

    device_handle_t stderr_dev;
    stderr_dev.type = DEVICE_TYPE_CHARDEV;
    stderr_dev.cdev = kprintf_get_cdev();
    if (!stderr_dev.cdev)
    {
        process_free(proc);
        return NULL;
    }

    proc->stderr = stream_create_driver(stderr_dev, NULL, stderr_write);
    if (!proc->stderr)
    {
        process_free(proc);
        return NULL;
    }

    proc->pid = current_pid++;

    return proc;
}

void process_free(process_t *proc)
{
    if (!proc)
    {
        return;
    }

    if (proc->stdin)
    {
        stream_free(proc->stdin);
    }
    if (proc->stdout)
    {
        stream_free(proc->stdout);
    }
    if (proc->stderr)
    {
        stream_free(proc->stderr);
    }

    if (proc->elf)
    {
        elf_free(proc->elf);
    }
    if (proc->pml4)
    {
        pmm_free((uint64_t *)proc->pml4);
    }
    if (proc->data_pages)
    {
        for (size_t i = 0; i < proc->num_data_pages; i++)
        {
            pmm_free(proc->data_pages[i]);
        }
        kfree(proc->data_pages);
    }
    if (proc->task && proc->task->stack_pages)
    {
        for (size_t i = 0; i < proc->task->num_stack_pages; i++)
        {
            pmm_free(proc->task->stack_pages[i]);
        }
        kfree(proc->task->stack_pages);
    }
    if (proc->task)
    {
        kfree(proc->task);
    }

    for (int i = 0; i < PROCESS_MAX_HEAP_PAGES; i++)
    {
        if (proc->allocations[i])
        {
            pmm_free(proc->allocations[i]);
        }
    }

    kfree(proc);
}

process_t *proc_head = NULL;
process_t *current_proc = NULL;

int process_register(process_t *proc)
{
    if (!proc_head)
    {
        proc_head = proc;
        return 0;
    }

    process_t *tail = NULL;
    for (tail = proc_head; tail->next != NULL; tail = tail->next);
    if (!tail)
    {
        return -ECORRUPT;
    }

    tail->next = proc;

    return 0;
}

int process_unregister(process_t *proc)
{
    if (!proc_head)
    {
        return -ECORRUPT;
    }

    if (current_proc == proc)
    {
        current_proc = NULL;
    }

    for (process_t *tail = proc_head; tail->next != NULL; tail = tail->next)
    {
        if (tail->next == proc)
        {
            tail->next = proc->next;
            return 0;
        }
    }

    return -EINVARG; // not found
}

void task_execute(uint64_t rip, uint64_t rsp, uint64_t eflags, task_state_t *state);

int execute_next_process(void)
{
    if (!proc_head)
    {
        return -ECORRUPT;
    }

    if (!current_proc)
    {
        current_proc = proc_head;
    }
    else
    {
        current_proc = current_proc->next;
        if (!current_proc)
        {
            current_proc = proc_head;
        }
    }

    task_state_t state = current_proc->task->state; // needs to be copied because proc is allocated and not mapped in processes pml4

    int status = pml4_switch(current_proc->pml4);
    if (status < 0)
    {
        return status;
    }

    // TODO: execute global constructors
    task_execute(state.rip, state.rsp, 0x202, &state);

    return 0; // never executed
}

process_t *get_current_process(void)
{
    return current_proc;
}

void *process_allocate_page(process_t *proc)
{
    for (uint64_t i = 0; i < PROCESS_MAX_HEAP_PAGES; i++)
    {
        if (proc->allocations[i] != 0)
        {
            proc->allocations[i] = pmm_alloc();
            if (!proc->allocations[i])
            {
                return NULL;
            }
            if (pml4_map(proc->pml4, (void *)(PROCESS_HEAP_VADDR_BASE + i * PAGE_SIZE), proc->allocations[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
            {
                return NULL;
            }

            return (void *)(PROCESS_HEAP_VADDR_BASE + i * PAGE_SIZE);
        }
    }

    return NULL;
}

uint64_t process_open_file(process_t *proc, const char *path, uint8_t action)
{
    for (uint64_t i = 0; i < PROCESS_MAX_FILES; i++)
    {
        if (proc->files[i] != 0)
        {
            proc->files[i] = vfs_open(path, action);
            if (!proc->files[i])
            {
                return 0;
            }

            return i+3;
        }
    }

    return 0;
}

int process_close_file(process_t *proc, uint64_t id)
{
    if (id < 3)
    {
        return -EINVARG;
    }
    if (proc->files[id-3] == NULL)
    {
        return -EINVARG;
    }

    int status = vfs_close(proc->files[id-3]);
    if (status < 0)
    {
        return status;
    }

    proc->files[id-3] = NULL;
    return 0;
}

file_node_t *process_get_file(process_t *proc, uint64_t id)
{
    if (id < 3)
    {
        return NULL;
    }

    return proc->files[id-3];
}

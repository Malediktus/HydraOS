#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

extern int __kernel_start;
extern int __kernel_end;

process_t *process_create(const char *path)
{
    process_t *proc = kmalloc(sizeof(process_t));
    if (!proc)
    {
        return NULL;
    }

    proc->elf = elf_load(path);
    if (!proc->elf)
    {
        kfree(proc);
        return NULL;
    }

    proc->task = kmalloc(sizeof(task_t));
    if (!proc->task)
    {
        elf_free(proc->elf);
        kfree(proc);
        return NULL;
    }

    memset(&proc->task->state, 0, sizeof(task_state_t));

    strncpy(proc->path, path, MAX_PATH);
    proc->pml4 = pmm_alloc();
    if (!proc->pml4)
    {
        elf_free(proc->elf);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    proc->task->parent = proc;

    for (uintptr_t i = (uintptr_t)&__kernel_start; i < (uintptr_t)&__kernel_end; i += PAGE_SIZE)
    {
        if (pml4_map(proc->pml4, (void *)i, (void *)i, PAGE_PRESENT | PAGE_WRITABLE) < 0)
        {
            elf_free(proc->elf);
            pmm_free((uint64_t *)proc->pml4);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }
    }

    proc->task->num_stack_pages = PROCESS_STACK_SIZE / PAGE_SIZE;
    proc->task->stack_pages = kmalloc(proc->task->num_stack_pages);
    if (!proc->task->stack_pages)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    for (size_t i = 0; i < proc->task->num_stack_pages; i++)
    {
        proc->task->stack_pages[i] = pmm_alloc();
        if (!proc->task->stack_pages[i])
        {
            elf_free(proc->elf);
            pmm_free((uint64_t *)proc->pml4);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }

        if (pml4_map(proc->pml4, (void *)(PROCESS_STACK_VADDR_BASE + (PROCESS_STACK_SIZE - i * PAGE_SIZE)), proc->task->stack_pages[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            elf_free(proc->elf);
            pmm_free((uint64_t *)proc->pml4);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }
    }

    if (elf_setup_exec(proc->elf, proc) < 0)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        for (size_t i = 0; i < proc->task->num_stack_pages; i++)
        {
            pmm_free(proc->task->stack_pages[i]);
        }
        kfree(proc->task->stack_pages);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    proc->task->state.rsp = PROCESS_STACK_VADDR_BASE + PROCESS_STACK_SIZE;
    proc->next = NULL;

    return proc;
}

int process_free(process_t *proc)
{
    for (size_t i = 0; i < proc->num_data_pages; i++)
    {
        pmm_free(proc->data_pages[i]);
    }

    kfree(proc->data_pages);
    elf_free(proc->elf);
    pmm_free((uint64_t *)proc->pml4);
    for (size_t i = 0; i < proc->task->num_stack_pages; i++)
    {
        pmm_free(proc->task->stack_pages[i]);
    }
    kfree(proc->task->stack_pages);
    kfree(proc->task);
    kfree(proc);

    return 0;
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
        return -1;
    }

    tail->next = proc;

    return 0;
}

int process_unregister(process_t *proc)
{
    if (!proc_head)
    {
        proc_head = proc;
        return -1;
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

    return -1; // not found
}

void task_execute(uint64_t rip, uint64_t rsp, uint64_t eflags, task_state_t *state);

int execute_next_process(void)
{
    if (!proc_head)
    {
        return -1;
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

    if (pml4_switch(current_proc->pml4) < 0)
    {
        return -1;
    }

    // TODO: execute global constructors
    task_execute(state.rip, state.rsp, 0x202, &state);

    return 0; // never executed
}

process_t *get_current_process(void)
{
    return current_proc;
}

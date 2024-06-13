#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

extern int __kernel_start;
extern int __kernel_end;

process_t *process_start(const char *path)
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
    proc->task->stack = pmm_alloc();
    if (!proc->task->stack)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    for (uintptr_t i = (uintptr_t)&__kernel_start; i < (uintptr_t)&__kernel_end; i += PAGE_SIZE)
    {
        if (pml4_map(proc->pml4, (void *)i, (void *)i, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            elf_free(proc->elf);
            pmm_free((uint64_t *)proc->pml4);
            pmm_free(proc->task->stack);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }
    }

    if (pml4_map(proc->pml4, (void *)(PROCESS_STACK_VADDR - PAGE_SIZE), proc->task->stack, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        pmm_free(proc->task->stack);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    if (pml4_map(proc->pml4, (void *)PROCESS_STACK_VADDR, proc->task->stack, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        pmm_free(proc->task->stack);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    if (elf_setup_exec(proc->elf, proc) < 0)
    {
        elf_free(proc->elf);
        pmm_free((uint64_t *)proc->pml4);
        pmm_free(proc->task->stack);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

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
    pmm_free(proc->task->stack);
    kfree(proc->task);
    kfree(proc);

    return 0;
}

void task_execute(void);

process_t *current_process = NULL;

int execute_process(process_t *proc)
{
    current_process = proc;
    if (pml4_switch(proc->pml4) < 0)
    {
        return -1;
    }

    task_execute();

    return 0; // never executed
}

process_t *get_current_process(void)
{
    return current_process;
}

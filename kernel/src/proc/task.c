#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

extern int __kernel_start;
extern int __kernel_end;

process_t *process_start(const char *path)
{
    file_node_t *node = vfs_open(path, OPEN_ACTION_READ);
    if (!node)
    {
        return NULL;
    }

    size_t num_data_pages = (node->filesize + (PAGE_SIZE - 1)) / PAGE_SIZE;
    void **data_pages = kmalloc(num_data_pages);

    for (size_t i = 0; i < num_data_pages; i++)
    {
        uint8_t *buf = pmm_alloc();
        if (!buf)
        {
            vfs_close(node);
            return NULL;
        }

        data_pages[i] = buf;

        size_t remainder = node->filesize - (i * PAGE_SIZE);
        if (remainder > PAGE_SIZE)
        {
            remainder = PAGE_SIZE;
        }

        if (vfs_read(node, remainder, buf) < 0)
        {
            kfree(buf);
            vfs_close(node);
            return NULL;
        }
    }

    if (vfs_close(node) < 0)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        return NULL;
    }

    process_t *proc = kmalloc(sizeof(process_t));
    if (!proc)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        return NULL;
    }

    proc->task = kmalloc(sizeof(task_t));
    if (!proc->task)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        kfree(proc);
        return NULL;
    }

    strncpy(proc->path, path, MAX_PATH);
    proc->pml4 = pmm_alloc();
    if (!proc->pml4)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    proc->data_pages = data_pages;
    proc->task->parent = proc;
    proc->task->stack = pmm_alloc();
    if (!proc->task->stack)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        pmm_free((uint64_t *)proc->pml4);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    for (uintptr_t i = (uintptr_t)&__kernel_start; i < (uintptr_t)&__kernel_end; i += PAGE_SIZE)
    {
        if (pml4_map(proc->pml4, (void *)i, (void *)i, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            for (size_t i = 0; i < num_data_pages; i++)
            {
                pmm_free(data_pages[i]);
            }
            kfree(data_pages);
            pmm_free((uint64_t *)proc->pml4);
            pmm_free(proc->task->stack);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }
    }

    for (size_t i = 0; i < num_data_pages; i++)
    {
        if (pml4_map(proc->pml4, (void *)(PROCESS_DATA_VADDR + i * PAGE_SIZE), proc->data_pages[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
        {
            for (size_t i = 0; i < num_data_pages; i++)
            {
                pmm_free(data_pages[i]);
            }
            kfree(data_pages);
            pmm_free((uint64_t *)proc->pml4);
            pmm_free(proc->task->stack);
            kfree(proc->task);
            kfree(proc);
            return NULL;
        }
    }

    if (pml4_map(proc->pml4, (void *)PROCESS_STACK_VADDR, proc->task->stack, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
    {
        for (size_t i = 0; i < num_data_pages; i++)
        {
            pmm_free(data_pages[i]);
        }
        kfree(data_pages);
        pmm_free((uint64_t *)proc->pml4);
        pmm_free(proc->task->stack);
        kfree(proc->task);
        kfree(proc);
        return NULL;
    }

    return proc;
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

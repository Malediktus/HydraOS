#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/fs/vfs.h>

#define PROCESS_DATA_VADDR 0x400000
#define PROCESS_STACK_VADDR 0x800000
#define PROCESS_STACK_SIZE 4096

struct _process;

typedef struct _task
{
    struct _process *parent;
    void *stack; // physical addresses
} task_t;

typedef struct _process
{
    task_t *task;

    char path[MAX_PATH];
    page_table_t *pml4;
    void **data_pages; // physical addresses
} process_t;

void syscall_init(void);

process_t *process_start(const char *path);
int execute_process(process_t *proc);
process_t *get_current_process(void);

#endif
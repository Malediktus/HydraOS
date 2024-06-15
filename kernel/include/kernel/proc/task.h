#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/elf.h>

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
    elf_file_t *elf;

    char path[MAX_PATH];
    page_table_t *pml4;
    size_t num_data_pages;
    void **data_pages; // physical addresses
} process_t;

void syscall_init(void);

process_t *process_start(const char *path);
int process_free(process_t *proc);
int execute_process(process_t *proc);
process_t *get_current_process(void);

#endif
#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/elf.h>

#define PROCESS_STACK_VADDR_BASE 0x800000
#define PROCESS_STACK_SIZE 4096 * 3

typedef struct
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, rsp;
} __attribute__((packed)) task_state_t;

struct _process;

typedef struct _task
{
    struct _process *parent;
    void **stack_pages; // physical addresses
    size_t num_stack_pages;
    task_state_t state;
} task_t;

typedef struct _process
{
    task_t *task;
    elf_file_t *elf;

    char path[MAX_PATH];
    page_table_t *pml4;
    void **data_pages; // physical addresses
    size_t num_data_pages;

    struct _process *next;
} process_t;

void syscall_init(void);

process_t *process_create(const char *path);
int process_free(process_t *proc);

int process_register(process_t *proc);
int process_unregister(process_t *proc);
int execute_next_process(void);
process_t *get_current_process(void);

#endif
#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>

extern page_table_t *kernel_pml4;

void syscall_handler(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5, uint64_t p6)
{
    uint64_t syscall_num;
    __asm__ volatile ("mov %%rax, %0" : "=r" (syscall_num)); // syscall num is stored in rax

    if (pml4_switch(kernel_pml4) < 0)
    {
        // TODO: panic
        while (1);
    }

    kprintf("syscall number: %d\n", syscall_num);

    process_t *proc = get_current_process();
    if (!proc)
    {
        // TODO: panic
        while (1);
    }

    if (pml4_switch(proc->pml4) < 0)
    {
        // TODO: panic
        while (1);
    }
}

#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/kprintf.h>
#include <kernel/proc/task.h>

extern page_table_t *kernel_pml4;

int64_t syscall_handler(uint64_t num, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5)
{
    if (pml4_switch(kernel_pml4) < 0)
    {
        // TODO: panic
        while (1);
    }

    kprintf("syscall number %d from %p, arg0 %d, arg1 %d, arg2 %d, arg3 %d, arg4 %d, arg5 %d\n", num, get_current_process(), arg0, arg1, arg2, arg3, arg4, arg5);

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

    return 3;
}

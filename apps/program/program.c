#include <hydra/kernel.h>

int main(void)
{
    int64_t pid = syscall(2, 0, 0, 0, 0, 0, 0);
    if (pid < 0)
    {
        syscall(1, 1, (uintptr_t)"Failed to fork process!\n", 24, 0, 0, 0);
        return 0;
    }

    if (pid == 0)
    {
        syscall(1, 1, (uintptr_t)"Forked process!\n", 16, 0, 0, 0);
        while (1)
        {
            char ascii = 0;
            syscall(0, 0, (uint64_t)&ascii, 1, 0, 0, 0);
            if (ascii != 0)
            {
                break;
            }
        }

        return 0;
    }

    while (syscall(4, pid, 0, 0, 0, 0, 0) == pid);

    uint64_t bytes = syscall(1, 1, (uintptr_t)"Hello User World!\n", 18, 0, 0, 0);

    while (1)
    {
        char ascii = 0;
        syscall(0, 0, (uint64_t)&ascii, 1, 0, 0, 0);
        if (ascii == 0)
        {
            continue;
        }

        syscall(1, 1, (uint64_t)&ascii, 1, 0, 0, 0);
    }

    while (1);
    return 0;
}

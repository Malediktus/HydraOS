#include <hydra/kernel.h>

int main(void)
{
    syscall(1, 1, (uintptr_t)"Hello User World!\n", 18, 0, 0, 0);

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

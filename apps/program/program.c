#include <hydra/kernel.h>
#include <stdio.h>

int main(void)
{
    int64_t pid = syscall_fork();
    if (pid < 0)
    {
        syscall_write(1, "Failed to fork process!\n", 24);
        return 0;
    }

    if (pid == 0)
    {
        syscall_write(1, "Forked process! Press enter to continue... \n", 43);
        while (fgetc(stdin) != '\n');

        return 0;
    }

    while (syscall_ping(pid) == pid);

    syscall_write(1, "Hello User World!\n", 18);

    while (1)
    {
        char ascii = fgetc(stdin);
        if (ascii == 0)
        {
            continue;
        }

        syscall_write(1, &ascii, 1);
    }

    return 0;
}

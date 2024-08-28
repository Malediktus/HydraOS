#include <hydra/kernel.h>
#include <stdio.h>

int main(void)
{
    int64_t pid = syscall_fork();
    if (pid < 0)
    {
        fputs("Failed to fork process!\n", stdout);
        return 0;
    }

    if (pid == 0)
    {
        fputs("Forked process! Press enter to continue... \n", stdout);
        while (fgetc(stdin) != '\n');

        return 0;
    }

    while (syscall_ping(pid) == pid);

    fputs("Hello User World!\n", stdout);

    while (1)
    {
        char ascii = fgetc(stdin);
        if (ascii == 0)
        {
            continue;
        }

        fputc(ascii, stdout);
    }

    return 0;
}

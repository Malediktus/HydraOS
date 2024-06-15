#include <h_syscall.h>

int main(void)
{
    int64_t res = syscall(5, 3, 2, 1, 0, 4, 6);
    for (int64_t i = 0; i < res; i++)
    {
        syscall(i, 0, 0, 0, 0, 0, 0);
    }

    while (1);

    return 0;
}

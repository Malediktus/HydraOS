#include <h_syscall.h>
#include <stddef.h>

void read(uint64_t stream, uint8_t *buf, size_t len)
{
    syscall(0, stream, (uint64_t)buf, len, 0, 0, 0);
}

void write(uint64_t stream, const uint8_t *buf, size_t len)
{
    syscall(1, stream, (uint64_t)buf, len, 0, 0, 0);
}

int main(void)
{
    const char *str = "Hello User World!\n";
    write(1, str, 18);

    uint8_t c = 0;
    while (1)
    {
        read(0, &c, 1);
        if (c != 0)
        {
            write(1, &c, 1);
        }
    }

    while (1);
    return 0;
}

#include <hydra/kernel.h>

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
{
    uint64_t result;
    asm volatile(
        "syscall"
        :   "=a"(result)
        :   "a"(num),
            "D"(arg1),
            "S"(arg2),
            "d"(arg3),
            "r"(arg5),
            "r"(arg6),
            "r"(arg4)
        : "rcx", "r11", "memory"
    );

    return result;
}

uint64_t syscall_read(uint64_t stream, uint8_t *data, size_t size)
{
    return syscall(_SYSCALL_READ, stream, (uint64_t)data, size, 0, 0, 0);
}

uint64_t syscall_write(uint64_t stream, const uint8_t *data, size_t size)
{
    return syscall(_SYSCALL_WRITE, stream, (uint64_t)data, size, 0, 0, 0);
}

uint64_t syscall_fork(void)
{
    return syscall(_SYSCALL_FORK, 0, 0, 0, 0, 0, 0);
}

void syscall_exit(uint32_t result)
{
    syscall(_SYSCALL_EXIT, result, 0, 0, 0, 0, 0);
}

uint64_t syscall_ping(uint64_t pid)
{
    return syscall(_SYSCALL_PING, pid, 0, 0, 0, 0, 0);
}

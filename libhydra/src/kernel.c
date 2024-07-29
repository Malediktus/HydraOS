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
            "r"(arg4),
            "r"(arg5),
            "r"(arg6)
        : "rcx", "r11", "memory"
    );

    return result;
}

#ifndef _KERNEL_H
#define _KERNEL_H 1

#include <stdint.h>
#include <stddef.h>

#define _SYSCALL_READ 0
#define _SYSCALL_WRITE 1
#define _SYSCALL_FORK 2
#define _SYSCALL_EXIT 3
#define _SYSCALL_PING 4

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

uint64_t syscall_read(uint64_t stream, uint8_t *data, size_t size);
uint64_t syscall_write(uint64_t stream, const uint8_t *data, size_t size);
uint64_t syscall_fork(void);
void syscall_exit(uint32_t result);
uint64_t syscall_ping(uint64_t pid);

#endif
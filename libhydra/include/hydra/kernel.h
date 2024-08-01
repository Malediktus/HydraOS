#ifndef _KERNEL_H
#define _KERNEL_H 1

#include <stdint.h>
#include <stddef.h>

#define _SYSCALL_DREAD 0
#define _SYSCALL_DWRITE 1
#define _SYSCALL_FORK 2
#define _SYSCALL_ALLOC 3
#define _SYSCALL_FOPEN 4
#define _SYSCALL_FCLOSE 5
#define _SYSCALL_FREAD 6
#define _SYSCALL_FWRITE 7

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

uint64_t syscall_read(uint64_t stream, uint8_t *data, size_t size);
uint64_t syscall_write(uint64_t stream, const uint8_t *data, size_t size);

#endif
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>

int64_t syscall(uint64_t num, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5);

#endif
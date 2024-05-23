#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);

int atoui(char *s);

#endif
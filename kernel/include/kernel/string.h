#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);

void *memset(void *dest, register int val, register size_t len);
void memcpy(void *dest, void *src, size_t len);

int atoui(char *s);

int isdigit(char c);
int isspace(char c);
int islower(char c);
int isupper(char c);
int isalpha(char c);

long strtol(const char *restrict nptr, char **restrict endptr, int base);
char *strchr(const char *p, int ch);

#endif
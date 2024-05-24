#include <kernel/string.h>

size_t strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s)
        ;

    return (s - str);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

void *memset(void *dest, register int val, register size_t len)
{
    register unsigned char *ptr = (unsigned char *)dest;
    while (len-- > 0)
    {
        *ptr++ = val;
    }
    return dest;
}

void memcpy(void *dest, void *src, size_t len)
{
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for (size_t i = 0; i < len; i++)
    {
        cdest[i] = csrc[i];
    }
}

int atoui(char *s)
{
    int acum = 0;
    while ((*s >= '0') && (*s <= '9'))
    {
        acum = acum * 10;
        acum = acum + (*s - 48);
        s++;
    }
    return (acum);
}

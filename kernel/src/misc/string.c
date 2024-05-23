#include <kernel/string.h>

size_t strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s);
    
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

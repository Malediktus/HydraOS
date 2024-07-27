#include <string.h>

void *memset(char *s, int c, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        s[i] = c;
    }

    return s;
}

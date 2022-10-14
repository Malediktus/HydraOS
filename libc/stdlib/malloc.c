#include <stddef.h>

#if defined(__is_libk)
#include <kernel/memory/kheap.h>
#endif

void *malloc(size_t size)
{
#if defined(__is_libk)
    return kzalloc(size);
#else
    // TODO: Implement process allocations
    return NULL;
#endif
}
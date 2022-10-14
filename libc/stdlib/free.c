#if defined(__is_libk)
#include <kernel/memory/kheap.h>
#endif

void free(void *ptr)
{
#if defined(__is_libk)
    kfree(ptr);
#else
    // TODO: Implement process allocations
#endif
}
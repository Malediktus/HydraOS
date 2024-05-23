#include <kernel/kmm.h>
#include <kernel/pmm.h>
#include <stdbool.h>

#define MAX_LEVEL 12

typedef struct buddy_node
{
    size_t size;
    bool free;
} buddy_node_t;

buddy_node_t *buddy_node_next(buddy_node_t *node)
{
    return (buddy_node_t *)((uintptr_t)node + node->size);
}

buddy_node_t *buddy_node_split(buddy_node_t *node, size_t size)
{
    if (!node || size == 0)
    {
        return NULL;
    }

    while (size < node->size)
    {
        size_t new_size = node->size >> 1;
        node->size = new_size;
        node = buddy_node_next(node);
        node->size = new_size;
        node->free = true;
    }

    if (size <= node->size)
    {
        return node;
    }

    return NULL;
}

buddy_node_t *buddy_node_find_best(buddy_node_t *head, buddy_node_t *tail, size_t size)
{
    buddy_node_t *res = NULL;
    buddy_node_t *node = head;
    buddy_node_t *buddy = buddy_node_next(node);

    if (buddy == tail && node->free)
    {
        return buddy_node_split(node, size);
    }

    while (node < tail && buddy < tail)
    {
        if (node->free && buddy->free && node->size == buddy->size)
        {
            node->size <<= 1;
            if (size <= node->size && (res == NULL || node->size <= res->size))
            {
                res = node;
            }

            node = buddy_node_next(buddy);
            if (node < tail)
            {
                buddy = buddy_node_next(node);
            }
            continue;
        }

        if (node->free && size <= node->size &&
            (res == NULL || node->size <= res->size))
        {
            res = node;
        }

        if (buddy->free && size <= buddy->size &&
            (res == NULL || buddy->size < res->size))
        {
            res = buddy;
        }

        if (node->size <= buddy->size)
        {
            node = buddy_node_next(buddy);
            if (node < tail)
            {
                buddy = buddy_node_next(node);
            }
        }
        else
        {
            node = buddy;
            buddy = buddy_node_next(buddy);
        }
    }

    if (res != NULL)
    {
        return buddy_node_split(res, size);
    }

    return NULL;
}

buddy_node_t *head;
buddy_node_t *tail;
size_t alignment;

int kmm_init(void *arena, size_t size, size_t _alignment)
{
    if (!arena || (size & (size - 1)) != 0 || (_alignment & (_alignment - 1)) != 0)
    {
        return -1;
    }

    if (_alignment < sizeof(buddy_node_t))
    {
        _alignment = sizeof(buddy_node_t);
    }

    if (((uintptr_t)arena % _alignment) != 0)
    {
        return -1;
    }

    head = arena;
    head->size = size;
    head->free = true;

    tail = buddy_node_next(head);

    alignment = _alignment;

    return 0;
}

size_t align_forward_size(size_t ptr, size_t align)
{
    size_t a, p, modulo;

    a = align;
    p = ptr;
    modulo = p & (a - 1);
    if (modulo != 0)
    {
        p += a - modulo;
    }
    return p;
}

size_t buddy_node_size_required(size_t size)
{
    size_t actual_size = alignment;

    size += sizeof(buddy_node_t);
    size = align_forward_size(size, alignment);

    while (size > actual_size)
    {
        actual_size <<= 1;
    }

    return actual_size;
}

void buddy_node_coalescence(buddy_node_t *head, buddy_node_t *tail)
{
    for (;;)
    {
        // Keep looping until there are no more buddies to coalesce

        buddy_node_t *block = head;
        buddy_node_t *buddy = buddy_node_next(block);

        bool no_coalescence = true;
        while (block < tail && buddy < tail)
        { // make sure the buddies are within the range
            if (block->free && buddy->free && block->size == buddy->size)
            {
                // Coalesce buddies into one
                block->size <<= 1;
                block = buddy_node_next(block);
                if (block < tail)
                {
                    buddy = buddy_node_next(block);
                    no_coalescence = false;
                }
            }
            else if (block->size < buddy->size)
            {
                // The buddy block is split into smaller blocks
                block = buddy;
                buddy = buddy_node_next(buddy);
            }
            else
            {
                block = buddy_node_next(buddy);
                if (block < tail)
                {
                    // Leave the buddy block for the next iteration
                    buddy = buddy_node_next(block);
                }
            }
        }

        if (no_coalescence)
        {
            return;
        }
    }
}

void *buddy_allocator_alloc(size_t size)
{
    if (size != 0)
    {
        size_t actual_size = buddy_node_size_required(size);

        buddy_node_t *found = buddy_node_find_best(head, tail, actual_size);
        if (found == NULL)
        {
            // Try to coalesce all the free buddy blocks and then search again
            buddy_node_coalescence(head, tail);
            found = buddy_node_find_best(head, tail, actual_size);
        }

        if (found != NULL)
        {
            found->free = false;
            return (void *)((char *)found + alignment);
        }

        // Out of memory (possibly due to too much internal fragmentation)
    }

    return NULL;
}

void *kmalloc(size_t size)
{
    return buddy_allocator_alloc(size);
}

void kfree(void *ptr)
{
    if (!ptr)
    {
        return;
    }

    buddy_node_t *node;
                
    node = (buddy_node_t *)((char *)ptr - alignment);
    node->free = true;
        
    buddy_node_coalescence(head, tail);
}

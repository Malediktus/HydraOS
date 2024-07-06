#ifndef _KERNEL_INPUTDEV_H
#define _KERNEL_INPUTDEV_H

#include <stdint.h>
#include <kernel/status.h>

#define IPACKET_NULL 0
#define IPACKET_KEYDOWN 1
#define IPACKET_KEYREPEAT 2
#define IPACKET_KEYUP 3

#define MODIFIER_SHIFT 1<<0
#define MODIFIER_CTRL 1<<1
#define MODIFIER_ALT 1<<2
#define MODIFIER_CAPS_LOCK 1<<3

typedef struct
{
    uint8_t type;
    uint8_t modifier;
    uint8_t scancode;
    int deltaX;
    int deltaY;
} inputpacket_t;

typedef struct _inputdev
{
    int (*poll)(inputpacket_t *, struct _inputdev *);
    int (*free)(struct _inputdev *);
    uint32_t references;
} inputdev_t;

inputdev_t *inputdev_new_ref(inputdev_t *idev);
int inputdev_free_ref(inputdev_t *idev);

int inputdev_poll(inputpacket_t *packet, inputdev_t *idev);

#endif
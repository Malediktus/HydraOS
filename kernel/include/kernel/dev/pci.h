#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H

#include <kernel/dev/chardev.h>
#include <kernel/status.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_BARS 6
#define MAX_BUS 256
#define MAX_DEVICE 32
#define MAX_FUNCTION 8
#define MAX_PCI_DEVICES (MAX_BUS * MAX_DEVICE * MAX_FUNCTION)

#define PCI_CLASS_DISPLAY_CONTROLLER 0x03
#define PCI_CLASS_MASS_STORAGE_CONTROLLER 0x01

#define PCI_SUBCLASS_VGA_COMP_CONTROLLER 0x00
#define PCI_SUBCLASS_IDE_CONTROLLER 0x01

enum bar_type
{
    BAR_TYPE_MEMORY_MAPPING = 0,
    BAR_TYPE_INPUT_OUTPUT = 1
};

typedef struct
{
    bool prefetchable;
    uint32_t address;
    uint32_t size;
    enum bar_type type;
} base_address_register_t;

typedef struct
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass_code;
    uint8_t prog_if;
    uint8_t header_type;
    base_address_register_t bars[MAX_BARS];
} pci_device_t;

int pci_init(void);
void pci_free(void);

pci_device_t *pci_get_device(uint64_t id);

#endif
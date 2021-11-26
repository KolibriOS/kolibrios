#include <stdio.h>
#include "umka.h"

struct devices_dat_entry {
    uint8_t fun:3;
    uint8_t dev:5;
    uint8_t bus;
    uint16_t pad1;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t irq;
    uint32_t pad2;
};

STDCALL void
dump_devices_dat_iter(struct pci_dev *node, void *arg) {
    FILE *f = arg;
    if (node->gsi) {
        fwrite(&(struct devices_dat_entry){.dev = node->dev,
                                           .fun = node->fun,
                                           .bus = node->parent->bus,
                                           .vendor_id = node->vendor_id,
                                           .device_id = node->device_id,
                                           .irq = node->gsi},
              1, sizeof(struct devices_dat_entry), f);
    }
}


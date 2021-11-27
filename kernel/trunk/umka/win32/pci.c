#include "pci.h"

__attribute__((stdcall)) uint32_t pci_read(uint32_t bus, uint32_t dev,
                                           uint32_t fun, uint32_t offset,
                                           size_t len) {
	printf("STUB: %s() -> 0", __func__);
    return 0;
}


#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

int pci_bus_read_config_byte (struct pci_bus *bus, u32 devfn,
                              int pos, u8 *value)
{
//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead8(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}

int pci_bus_read_config_word (struct pci_bus *bus, u32 devfn,
                              int pos, u16 *value)
{
    if ( pos & 1)
        return PCIBIOS_BAD_REGISTER_NUMBER;

//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead16(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}


int pci_bus_read_config_dword (struct pci_bus *bus, u32 devfn,
                               int pos, u32 *value)
{
    if ( pos & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;

//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead32(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}

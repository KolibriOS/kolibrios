
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>


int pci_read_config_byte(struct pci_dev *dev, int where, u8 *val)
{
    *val = PciRead8(dev->busnr, dev->devfn, where);
    return 0;
}

int pci_read_config_word(struct pci_dev *dev, int where, u16 *val)
{

    if ( where & 1)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    *val = PciRead16(dev->busnr, dev->devfn, where);
    return 0;
}

int pci_read_config_dword(struct pci_dev *dev, int where, u32 *val)
{

    if ( where & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    *val = PciRead32(dev->busnr, dev->devfn, where);
    return 0;
}

int pci_write_config_byte(struct pci_dev *dev, int where, u8 val)
{
    PciWrite8(dev->busnr, dev->devfn, where, val);
    return 0;
};

int pci_write_config_word(struct pci_dev *dev, int where, u16 val)
{
    if ( where & 1)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    PciWrite16(dev->busnr, dev->devfn, where, val);
    return 0;
}

int pci_write_config_dword(struct pci_dev *dev, int where,
                     u32 val)
{
    if ( where & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    PciWrite32(dev->busnr, dev->devfn, where, val);
    return 0;
}


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
                              int pos, u16 *value)
{
    if ( pos & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;

//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead32(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}

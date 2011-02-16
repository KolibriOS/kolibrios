
#ifndef __DMDEV_H__
#define __DMDEV_H__


typedef struct
{
    struct list_head list;
    uint32_t  type;
    struct acpi_device *acpi_dev;
    struct pci_dev     *pci_dev;

}dmdev_t;


#endif /* __DMDEV_H_ */

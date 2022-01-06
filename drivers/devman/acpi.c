
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"
#include "acpi_bus.h"
#include "dmdev.h"

#define PREFIX "ACPI: "

#define ACPI_BUS_CLASS          "system_bus"
#define ACPI_BUS_HID            "KLBSYBUS"
#define ACPI_BUS_DEVICE_NAME    "System Bus"


#define ACPI_IS_ROOT_DEVICE(device)    (!(device)->parent)

LIST_HEAD(acpi_device_list);
LIST_HEAD(acpi_bus_id_list);
LIST_HEAD(dmdev_tree);


struct acpi_device_bus_id
{
    char bus_id[15];
    unsigned int instance_no;
    struct list_head node;
};


#define ACPI_NS_ROOT_PATH       "\\"
#define ACPI_NS_SYSTEM_BUS      "_SB_"

enum acpi_irq_model_id {
	ACPI_IRQ_MODEL_PIC = 0,
	ACPI_IRQ_MODEL_IOAPIC,
	ACPI_IRQ_MODEL_IOSAPIC,
	ACPI_IRQ_MODEL_PLATFORM,
	ACPI_IRQ_MODEL_COUNT
};

enum acpi_bus_removal_type {
    ACPI_BUS_REMOVAL_NORMAL = 0,
    ACPI_BUS_REMOVAL_EJECT,
    ACPI_BUS_REMOVAL_SUPRISE,
    ACPI_BUS_REMOVAL_TYPE_COUNT
};


#define PCI_MAX_DEVICES 32
#define PCI_MAX_PINS    4

#define IRQ_TABLE_ENTRIES   (PCI_MAX_DEVICES * PCI_MAX_PINS)

static int irqtable[IRQ_TABLE_ENTRIES];
static ACPI_HANDLE pci_root_handle;


#define  addr_offset(addr, off) \
    (addr_t)((addr_t)(addr) + (addr_t)(off))

//#define acpi_remap( addr ) \
//    (addr_t)((addr_t)(addr) + OS_BASE)

#define acpi_remap( addr ) MapIoMem((void*)(addr),4096, 0x01)

char* strdup(const char *str);
int sprintf(char *buf, const char *fmt, ...);

void print_pci_irqs();


struct acpi_device *acpi_root;

extern struct resource iomem_resource;
extern struct resource ioport_resource;

enum pic_mode
{
    IO_PIC  = 0,
    IO_APIC
};

static ACPI_STATUS
resource_to_addr(ACPI_RESOURCE *resource, ACPI_RESOURCE_ADDRESS64 *addr);

static void create_dm_list();

static void print_dm_list();

int write_device_dat(char *path);


static void set_pic_mode(enum pic_mode mode)
{
    ACPI_OBJECT arg1;
    ACPI_OBJECT_LIST args;
    ACPI_STATUS as;

    arg1.Type = ACPI_TYPE_INTEGER;
    arg1.Integer.Value = mode;
    args.Count = 1;
    args.Pointer = &arg1;

    as = AcpiEvaluateObject(ACPI_ROOT_OBJECT, "_PIC", &args, NULL);
    /*
     * We can silently ignore failure as it may not be implemented, ACPI should
     * provide us with correct information anyway
     */
    if (ACPI_SUCCESS(as))
        dbgprintf(PREFIX "machine set to %s mode\n", mode ? "APIC" : "PIC");
}



static bool pci_use_crs = false;

#define IORESOURCE_BUS      0x00001000


extern struct list_head acpi_pci_roots;

#define ACPI_PCI_ROOT_CLASS     "pci_bridge"
#define ACPI_PCI_ROOT_DEVICE_NAME   "PCI Root Bridge"

static ACPI_STATUS
get_root_bridge_busnr_callback(ACPI_RESOURCE *resource, void *data)
{
    struct resource *res = data;
    ACPI_RESOURCE_ADDRESS64 address;

    if (resource->Type != ACPI_RESOURCE_TYPE_ADDRESS16 &&
        resource->Type != ACPI_RESOURCE_TYPE_ADDRESS32 &&
        resource->Type != ACPI_RESOURCE_TYPE_ADDRESS64)
        return AE_OK;

    AcpiResourceToAddress64(resource, &address);
    if ((address.AddressLength > 0) &&
        (address.ResourceType == ACPI_BUS_NUMBER_RANGE)) {
        res->start = address.Minimum;
        res->end = address.Minimum + address.AddressLength - 1;
    }

    return AE_OK;
}



static ACPI_STATUS try_get_root_bridge_busnr(ACPI_HANDLE handle,
                         struct resource *res)
{
    ACPI_STATUS status;

    res->start = -1;
    status =
        AcpiWalkResources(handle, METHOD_NAME__CRS,
                get_root_bridge_busnr_callback, res);
    if (ACPI_FAILURE(status))
        return status;
    if (res->start == -1)
        return AE_ERROR;
    return AE_OK;
}


static void acpi_pci_bridge_scan(struct acpi_device *device)
{
    int status;
    struct acpi_device *child = NULL;

    if (device->flags.bus_address)
        if (device->parent && device->parent->ops.bind) {
            status = device->parent->ops.bind(device);
            if (!status) {
                list_for_each_entry(child, &device->children, node)
                    acpi_pci_bridge_scan(child);
            }
        }
}



struct pci_root_info
{
    struct acpi_device *bridge;
    char *name;
    unsigned int res_num;
    struct resource *res;
    struct pci_bus *bus;
    int busnum;
};


static ACPI_STATUS
resource_to_addr(ACPI_RESOURCE *resource, ACPI_RESOURCE_ADDRESS64 *addr)
{
    ACPI_STATUS status;
    struct acpi_resource_memory24 *memory24;
    struct acpi_resource_memory32 *memory32;
    struct acpi_resource_fixed_memory32 *fixed_memory32;

    memset(addr, 0, sizeof(*addr));
    switch (resource->Type) {
    case ACPI_RESOURCE_TYPE_MEMORY24:
        memory24 = &resource->Data.Memory24;
        addr->ResourceType = ACPI_MEMORY_RANGE;
        addr->Minimum = memory24->Minimum;
        addr->AddressLength = memory24->AddressLength;
        addr->Maximum = addr->Minimum + addr->AddressLength - 1;
        return AE_OK;
    case ACPI_RESOURCE_TYPE_MEMORY32:
        memory32 = &resource->Data.Memory32;
        addr->ResourceType = ACPI_MEMORY_RANGE;
        addr->Minimum = memory32->Minimum;
        addr->AddressLength = memory32->AddressLength;
        addr->Maximum = addr->Minimum + addr->AddressLength - 1;
        return AE_OK;
    case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
        fixed_memory32 = &resource->Data.FixedMemory32;
        addr->ResourceType = ACPI_MEMORY_RANGE;
        addr->Minimum = fixed_memory32->Address;
        addr->AddressLength = fixed_memory32->AddressLength;
        addr->Maximum = addr->Minimum + addr->AddressLength - 1;
        return AE_OK;
    case ACPI_RESOURCE_TYPE_ADDRESS16:
    case ACPI_RESOURCE_TYPE_ADDRESS32:
    case ACPI_RESOURCE_TYPE_ADDRESS64:
        status = AcpiResourceToAddress64(resource, addr);
        if (ACPI_SUCCESS(status) &&
            (addr->ResourceType == ACPI_MEMORY_RANGE ||
            addr->ResourceType == ACPI_IO_RANGE) &&
            addr->AddressLength > 0) {
            return AE_OK;
        }
        break;
    }
    return AE_ERROR;
}


static ACPI_STATUS
count_resource(ACPI_RESOURCE *acpi_res, void *data)
{
    struct pci_root_info *info = data;
    ACPI_RESOURCE_ADDRESS64 addr;
    ACPI_STATUS status;

    status = resource_to_addr(acpi_res, &addr);
    if (ACPI_SUCCESS(status))
        info->res_num++;
    return AE_OK;
}


static ACPI_STATUS setup_resource(ACPI_RESOURCE *acpi_res, void *data)
{
    struct pci_root_info *info = data;
    struct resource *res;
    struct acpi_resource_address64 addr;
    ACPI_STATUS status;
    unsigned long flags;
    u64 start, end;

    status = resource_to_addr(acpi_res, &addr);
    if (!ACPI_SUCCESS(status))
        return AE_OK;

    if (addr.ResourceType == ACPI_MEMORY_RANGE)
    {
        flags = IORESOURCE_MEM;
        if (addr.Info.Mem.Caching == ACPI_PREFETCHABLE_MEMORY)
            flags |= IORESOURCE_PREFETCH;
    }
    else if (addr.ResourceType == ACPI_IO_RANGE)
    {
        flags = IORESOURCE_IO;
    } else
        return AE_OK;

    start = addr.Minimum + addr.TranslationOffset;
    end = addr.Maximum + addr.TranslationOffset;

    res = &info->res[info->res_num];
    res->name = info->name;
    res->flags = flags;
    res->start = start;
    res->end = end;
    res->child = NULL;

    if (!pci_use_crs) {
        printk("host bridge window %pR (ignored)\n", res);
        return AE_OK;
    }

    info->res_num++;
    if (addr.TranslationOffset)
        dev_info(NULL, "host bridge window %pR "
             "(PCI address [%#llx-%#llx])\n",
             res, res->start - addr.TranslationOffset,
             res->end - addr.TranslationOffset);
    else
        dev_info(NULL,
             "host bridge window %pR\n", res);

    return AE_OK;
}



static void
get_current_resources(struct acpi_device *device, int busnum,
            int domain, struct pci_bus *bus)
{
    struct pci_root_info info;
    size_t size;

    char buf[64];

//    if (pci_use_crs)
//        pci_bus_remove_resources(bus);

    info.bridge = device;
    info.bus = bus;
    info.res_num = 0;
    AcpiWalkResources(device->handle, METHOD_NAME__CRS, count_resource,
                &info);
    if (!info.res_num)
        return;

    size = sizeof(*info.res) * info.res_num;
    info.res = kmalloc(size, GFP_KERNEL);
    if (!info.res)
        goto res_alloc_fail;

    sprintf(buf,"PCI Bus %04x:%02x", domain, busnum);

    info.name = strdup(buf);

    if (!info.name)
        goto name_alloc_fail;

    info.res_num = 0;
    AcpiWalkResources(device->handle, METHOD_NAME__CRS, setup_resource,
                &info);

    return;

name_alloc_fail:
    kfree(info.res);
res_alloc_fail:
    return;
}


struct pci_ops pci_root_ops = {
    .read = NULL,
    .write = NULL,
};


struct pci_bus*  pci_acpi_scan_root(struct acpi_pci_root *root)
{
    struct acpi_device *device = root->device;
    int domain = root->segment;
    int busnum = root->secondary.start;
    struct pci_bus *bus;
    struct pci_sysdata *sd;
	int node = 0;

    if (domain ) {
        printk(KERN_WARNING "pci_bus %04x:%02x: "
               "ignored (multiple domains not supported)\n",
               domain, busnum);
        return NULL;
    }

	node = -1;
    /* Allocate per-root-bus (not per bus) arch-specific data.
     * TODO: leak; this memory is never freed.
     * It's arguable whether it's worth the trouble to care.
     */
    sd = kzalloc(sizeof(*sd), GFP_KERNEL);
    if (!sd) {
        printk(KERN_WARNING "pci_bus %04x:%02x: "
               "ignored (out of memory)\n", domain, busnum);
        return NULL;
    }

    sd->domain = domain;
    sd->node = node;
    /*
     * Maybe the desired pci bus has been already scanned. In such case
     * it is unnecessary to scan the pci bus with the given domain,busnum.
     */
    bus = pci_find_bus(domain, busnum);
    if (bus) {
        /*
         * If the desired bus exits, the content of bus->sysdata will
         * be replaced by sd.
         */
        memcpy(bus->sysdata, sd, sizeof(*sd));
        kfree(sd);
    } else {
        bus = pci_create_bus(busnum, &pci_root_ops, sd);
        if (bus) {
            get_current_resources(device, busnum, domain, bus);
            bus->subordinate = pci_scan_child_bus(bus);
        }
    }

    if (!bus)
        kfree(sd);

    if (bus && node != -1) {
        printk("on NUMA node %d\n", node);
    }

    return bus;
}



static int acpi_pci_root_add(struct acpi_device *device)
{
    unsigned long long segment, bus;
    ACPI_STATUS status;
    int result;
    struct acpi_pci_root *root;
    ACPI_HANDLE handle;
    struct acpi_device *child;
    u32 flags, base_flags;

    root = kzalloc(sizeof(struct acpi_pci_root), GFP_KERNEL);
    if (!root)
        return -ENOMEM;

    segment = 0;
    status = acpi_evaluate_integer(device->handle, METHOD_NAME__SEG, NULL,
                       &segment);
    if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
        printk(KERN_ERR PREFIX "can't evaluate _SEG\n");
        result = -ENODEV;
        goto end;
    }

    /* Check _CRS first, then _BBN.  If no _BBN, default to zero. */
    root->secondary.flags = IORESOURCE_BUS;
    status = try_get_root_bridge_busnr(device->handle, &root->secondary);
    if (ACPI_FAILURE(status))
    {
        /*
         * We need both the start and end of the downstream bus range
         * to interpret _CBA (MMCONFIG base address), so it really is
         * supposed to be in _CRS.  If we don't find it there, all we
         * can do is assume [_BBN-0xFF] or [0-0xFF].
         */
        root->secondary.end = 0xFF;
        printk(KERN_WARNING PREFIX
               "no secondary bus range in _CRS\n");
        status = acpi_evaluate_integer(device->handle, METHOD_NAME__BBN,                           NULL, &bus);
        if (ACPI_SUCCESS(status))
            root->secondary.start = bus;
        else if (status == AE_NOT_FOUND)
            root->secondary.start = 0;
        else {
            printk(KERN_ERR PREFIX "can't evaluate _BBN\n");
            result = -ENODEV;
            goto end;
        }
    }

    INIT_LIST_HEAD(&root->node);
    root->device = device;
    root->segment = segment & 0xFFFF;
    strcpy(acpi_device_name(device), ACPI_PCI_ROOT_DEVICE_NAME);
    strcpy(acpi_device_class(device), ACPI_PCI_ROOT_CLASS);
    device->driver_data = root;

    /*
     * All supported architectures that use ACPI have support for
     * PCI domains, so we indicate this in _OSC support capabilities.
     */
//    flags = base_flags = OSC_PCI_SEGMENT_GROUPS_SUPPORT;
//    acpi_pci_osc_support(root, flags);

    /*
     * TBD: Need PCI interface for enumeration/configuration of roots.
     */

    /* TBD: Locking */
    list_add_tail(&root->node, &acpi_pci_roots);

    printk(KERN_INFO PREFIX "%s [%s] (domain %04x %pR)\n",
           acpi_device_name(device), acpi_device_bid(device),
           root->segment, &root->secondary);

    /*
     * Scan the Root Bridge
     * --------------------
     * Must do this prior to any attempt to bind the root device, as the
     * PCI namespace does not get created until this call is made (and
     * thus the root bridge's pci_dev does not exist).
     */

    root->bus = pci_acpi_scan_root(root);
    if (!root->bus) {
        printk(KERN_ERR PREFIX
                "Bus %04x:%02x not present in PCI namespace\n",
                root->segment, (unsigned int)root->secondary.start);
        result = -ENODEV;
        goto end;
    }

    /*
     * Attach ACPI-PCI Context
     * -----------------------
     * Thus binding the ACPI and PCI devices.
     */
    result = acpi_pci_bind_root(device);
    if (result)
        goto end;

    /*
     * PCI Routing Table
     * -----------------
     * Evaluate and parse _PRT, if exists.
     */
    status = AcpiGetHandle(device->handle, METHOD_NAME__PRT, &handle);
    if (ACPI_SUCCESS(status))
        result = acpi_pci_irq_add_prt(device->handle, root->bus);

    /*
     * Scan and bind all _ADR-Based Devices
     */
    list_for_each_entry(child, &device->children, node)
        acpi_pci_bridge_scan(child);

    return 0;

end:
    if (!list_empty(&root->node))
        list_del(&root->node);
    kfree(root);
    return result;
}


static const struct acpi_device_ids root_device_ids[] =
{
    {"PNP0A03", 0},
    {"",        0},
};

void acpi_init_pci(struct acpi_device *device)
{
    struct acpi_device *child;

    if ( !acpi_match_device_ids(device, root_device_ids) )
    {
        dbgprintf(PREFIX "PCI root %s\n", device->pnp.bus_id);
        acpi_pci_root_add(device);
    };

    list_for_each_entry(child, &device->children, node)
    {
        acpi_init_pci(child);
    };

};

uint32_t drvEntry(int action, char *cmdline)
{
    uint32_t retval;
    ACPI_STATUS status;
    int i;

    if(action != 1)
        return 0;

    status = AcpiInitializeSubsystem();
    if (status != AE_OK) {
          printf("AcpiInitializeSubsystem failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    status = AcpiInitializeTables (NULL, 16, FALSE);
    if (status != AE_OK) {
          printf("AcpiInitializeTables failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    status = AcpiLoadTables();
    if (status != AE_OK) {
          printf("AcpiLoadTables failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    // u32_t mode = ACPI_NO_HARDWARE_INIT | ACPI_NO_ACPI_ENABLE;

    status = AcpiEnableSubsystem(ACPI_NO_HANDLER_INIT | ACPI_NO_HARDWARE_INIT);
    if (status != AE_OK) {
        dbgprintf("AcpiEnableSubsystem failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (status))
    {
        dbgprintf("AcpiInitializeObjects failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    set_pic_mode(IO_APIC);
    acpi_scan();
    acpi_init_pci(acpi_root);
    print_pci_irqs();
    create_dm_list();
    print_dm_list();

    write_device_dat("/SYS/DRIVERS/DEVICES.DAT");

err:
    return 0;
};

char* strdup(const char *str)
{
    size_t len = strlen (str) + 1;
    char *copy = malloc(len);
    if (copy)
    {
        memcpy (copy, str, len);
    }
    return copy;
}


static void dm_add_pci_bus(struct pci_bus *bus)
{
    struct pci_bus *tbus;
    struct pci_dev *dev;
    dmdev_t        *dmdev;

    dmdev = (dmdev_t*)kzalloc(sizeof(dmdev_t),GFP_KERNEL);

//    INIT_LIST_HEAD(&dmdev->list);
//    dmdev->type = 1;
//    dmdev->acpi_dev = bus->self->acpi_dev;
//    dmdev->pci_dev = bus->self;
//    list_add_tail(&dmdev->list, &dmdev_tree);

    list_for_each_entry(dev, &bus->devices, bus_list)
    {
        dmdev = (dmdev_t*)kzalloc(sizeof(dmdev_t),GFP_KERNEL);

        INIT_LIST_HEAD(&dmdev->list);
        dmdev->type = 1;
        dmdev->acpi_dev = dev->acpi_dev;
        dmdev->pci_dev = dev;
        list_add_tail(&dmdev->list, &dmdev_tree);
    };

    list_for_each_entry(tbus, &bus->children, node)
    {
        dm_add_pci_bus(tbus);
    };

};

static ACPI_STATUS
count_dev_resources(ACPI_RESOURCE *acpi_res, void *data)
{
    (*(int*)data)++;
    return AE_OK;
}

static void dm_add_acpi(struct acpi_device *device)
{
    struct acpi_device *child;
    ACPI_DEVICE_INFO *info = NULL;
    ACPI_STATUS status;

    dmdev_t  *dmdev;
    uint32_t  res_num = 0;

    status = AcpiGetObjectInfo(device->handle, &info);

    if ( (status == AE_OK) && (info->Valid & ACPI_VALID_HID))
    {
        if( strcmp(info->HardwareId.String,"PNP0C0F") == 0)
        {
            kfree(info);
            return;
        };
    };

    kfree(info);

    if(device->pci_dev == NULL)
    {
        AcpiWalkResources(device->handle, METHOD_NAME__CRS,
                          count_dev_resources, &res_num);

        if(res_num != 0)
        {
            dmdev = (dmdev_t*)kzalloc(sizeof(dmdev_t),GFP_KERNEL);

            INIT_LIST_HEAD(&dmdev->list);
            dmdev->type = 0;
            dmdev->acpi_dev = device;
            dmdev->pci_dev = NULL;
            list_add_tail(&dmdev->list, &dmdev_tree);
        };
    };
    list_for_each_entry(child, &device->children, node)
    {
        dm_add_acpi(child);
    };
};

static void create_dm_list()
{
    struct acpi_pci_root *root;

    list_for_each_entry(root, &acpi_pci_roots, node)
    {
        struct pci_bus *pbus, *tbus;
        struct pci_dev *dev;

        pbus = root->bus;

        dm_add_pci_bus(pbus);
    };

    dm_add_acpi(acpi_root);
};

static void print_pci_resource(struct resource *res)
{
    if(res->flags !=0 )
    {
        if(res->flags & IORESOURCE_IO)
            dbgprintf("  IO range ");
        else if(res->flags & IORESOURCE_MEM)
            dbgprintf("  MMIO range ");
        dbgprintf("%x - %x\n", res->start, res->end);
    };
};

static ACPI_STATUS
print_acpi_resource(ACPI_RESOURCE *acpi_res, void *data)
{
    ACPI_RESOURCE_ADDRESS64 addr;
    ACPI_STATUS status;
    int i;

    switch (acpi_res->Type)
    {
        case ACPI_RESOURCE_TYPE_IRQ:
        {
            ACPI_RESOURCE_IRQ *irq_data = (ACPI_RESOURCE_IRQ*)&acpi_res->Data;
            dbgprintf(" IRQ %d\n", irq_data->Interrupts[0]);
        };
        break;

        case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
        {
            ACPI_RESOURCE_EXTENDED_IRQ *irq_data = (ACPI_RESOURCE_EXTENDED_IRQ*)&acpi_res->Data;
            dbgprintf(" IRQ %d\n", irq_data->Interrupts[0]);
        };
        break;

        case ACPI_RESOURCE_TYPE_DMA:
        {
            ACPI_RESOURCE_DMA *dma_data = (ACPI_RESOURCE_DMA*) &acpi_res->Data;
            for(i=0; i < dma_data->ChannelCount; i++)
            {
                dbgprintf(" DMA %s channel %d\n",
                          dma_data->Type == ACPI_TYPE_A ? "Type A":
                          dma_data->Type == ACPI_TYPE_B ? "Type B" :
                          dma_data->Type == ACPI_TYPE_F ? "Type F" : "",
                          dma_data->Channels[i]);
            }
        };
        break;

        case ACPI_RESOURCE_TYPE_IO:
        {
            ACPI_RESOURCE_IO *io_data = (ACPI_RESOURCE_IO*) &acpi_res->Data;

            dbgprintf(" IO range 0%x-0%x\n",io_data->Minimum,
                       io_data->Minimum+io_data->AddressLength-1);
        }
        break;

        case ACPI_RESOURCE_TYPE_FIXED_IO:
        {
            ACPI_RESOURCE_FIXED_IO *io_data = (ACPI_RESOURCE_FIXED_IO*) &acpi_res->Data;
            dbgprintf(" Fixed IO range 0%x-0%x\n",io_data->Address,
                       io_data->Address+io_data->AddressLength-1);
        };
        break;

        case ACPI_RESOURCE_TYPE_MEMORY24:
        case ACPI_RESOURCE_TYPE_MEMORY32:
        case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
        {
            ACPI_RESOURCE_ADDRESS64 addr64;
            resource_to_addr(acpi_res, &addr64);
            dbgprintf(" Memory range 0%x-0%x\n",
                       (uint32_t)addr64.Minimum, (uint32_t)addr64.Maximum);
        }
        break;

        case ACPI_RESOURCE_TYPE_ADDRESS16:
        case ACPI_RESOURCE_TYPE_ADDRESS32:
        case ACPI_RESOURCE_TYPE_ADDRESS64:
        {
            ACPI_RESOURCE_ADDRESS64 addr64;
            ACPI_STATUS status;

            status = AcpiResourceToAddress64(acpi_res, &addr64);
            if (ACPI_SUCCESS(status))
            {
                dbgprintf(" Address range 0%x-0%x\n",
                       (uint32_t)addr64.Minimum, (uint32_t)addr64.Maximum);
            }
        };
        break;
    };

    return AE_OK;
};


static void print_dm_list()
{
    struct pci_dev     *pcidev;
    struct acpi_device *acpidev;
    dmdev_t  *dmdev;
    uint32_t  i;

    dbgprintf("\nDevices:\n");

    list_for_each_entry(dmdev, &dmdev_tree, list)
    {
        switch(dmdev->type)
        {
            case 0:
               if(dmdev->acpi_dev != NULL)
               {
                    acpidev = dmdev->acpi_dev;
                    dbgprintf("\n%s\n", acpidev->pnp.bus_id);
                    AcpiWalkResources(acpidev->handle, METHOD_NAME__CRS,
                                      print_acpi_resource, NULL);
               };
               break;

            case 1:
               if(dmdev->pci_dev != NULL)
               {
                   pcidev = dmdev->pci_dev;
                   dbgprintf("\nPCI_%x_%x bus:%d devfn: %x\n",
                               pcidev->vendor, pcidev->device,
                               pcidev->busnr, pcidev->devfn);

                   for(i = 0; i < DEVICE_COUNT_RESOURCE; i++)
                       print_pci_resource(&pcidev->resource[i]);

                   if(pcidev->pin)
                       dbgprintf("  APIC IRQ: %d\n", acpi_get_irq(pcidev));
               };
               break;
        };
    };
};


typedef struct
{
    uint32_t  busaddr;
    uint32_t  devid;
    uint32_t  irq;
    uint32_t  unused;
}devinfo_t;

#pragma pack(push, 1)
typedef struct
{
  char sec;
  char min;
  char hour;
  char rsv;
}detime_t;

typedef struct
{
  char  day;
  char  month;
  short year;
}dedate_t;

typedef struct
{
  unsigned    attr;
  unsigned    flags;
  union
  {
     detime_t  ctime;
     unsigned  cr_time;
  };
  union
  {
     dedate_t  cdate;
     unsigned  cr_date;
  };
  union
  {
     detime_t  atime;
     unsigned  acc_time;
  };
  union
  {
     dedate_t  adate;
     unsigned  acc_date;
  };
  union
  {
     detime_t  mtime;
     unsigned  mod_time;
  };
  union
  {
     dedate_t  mdate;
     unsigned  mod_date;
  };
  unsigned    size;
  unsigned    size_high;
} FILEINFO;

#pragma pack(pop)


int write_device_dat(char *path)
{
    struct pci_dev   *pcidev;
    dmdev_t          *dmdev;
    devinfo_t        *data;
    int               writes;
    int               len;
    int i = 0;

    list_for_each_entry(dmdev, &dmdev_tree, list)
    {
        if(dmdev->type ==1)
        {
            if(dmdev->pci_dev != NULL)
            {
                pcidev = dmdev->pci_dev;
                if(pcidev->pin)
                    i++;
            };
        };
    };

    len = sizeof(devinfo_t)*i + 4;
    data = (devinfo_t*)malloc(len);

    i = 0;

    list_for_each_entry(dmdev, &dmdev_tree, list)
    {
        if(dmdev->type == 1)
        {

            if(dmdev->pci_dev != NULL)
            {
                pcidev = dmdev->pci_dev;
                if(pcidev->pin && (acpi_get_irq(pcidev) != -1) )
                {
                    data[i].busaddr = (pcidev->busnr<<8)|pcidev->devfn;
                    data[i].devid   = ((uint32_t)pcidev->device<<16) |
                                       pcidev->vendor;
                    data[i].irq     =  acpi_get_irq(pcidev);
                    data[i].unused  =  0;
                    i++;
                }
            };
        };
    };

    data[i].busaddr = -1;

    FILEINFO info;

    int offset = 0;

    if(get_fileinfo(path,&info))
    {
        if( create_file(path))
        {
            free(data);
            return false;
        }
    }
    else
        set_file_size(path, 0);

    write_file(path, data, 0, len, &writes);

    return true;
};

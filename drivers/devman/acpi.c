
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"
#include "acpi_bus.h"

#define PREFIX "ACPI: "

#define ACPI_BUS_CLASS          "system_bus"
#define ACPI_BUS_HID            "KLBSYBUS"
#define ACPI_BUS_DEVICE_NAME    "System Bus"


#define ACPI_IS_ROOT_DEVICE(device)    (!(device)->parent)

static LIST_HEAD(acpi_device_list);
static LIST_HEAD(acpi_bus_id_list);


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


struct acpi_device *acpi_root;

extern struct resource iomem_resource;
extern struct resource ioport_resource;

enum pic_mode
{
    IO_PIC  = 0,
    IO_APIC
};

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

void print_device_tree(struct acpi_device *device)
{
    struct acpi_device *child;

    dbgprintf("%s\n", device->pnp.bus_id);

    list_for_each_entry(child, &device->children, node)
    {
        print_device_tree(child);
    };
};


/*
int acpi_pci_bind_root(struct acpi_device *device)
{
    device->ops.bind = acpi_pci_bind;
    device->ops.unbind = acpi_pci_unbind;

    return 0;
}
*/

static bool pci_use_crs = false;

#define IORESOURCE_BUS      0x00001000

struct acpi_pci_root {
    struct list_head node;
    struct acpi_device * device;
    struct acpi_pci_id id;
    struct pci_bus *bus;
    u16 segment;
    struct resource secondary;      /* downstream bus range */

};

static LIST_HEAD(acpi_pci_roots);

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
    struct resource *root, *conflict;
    u64 start, end;

    status = resource_to_addr(acpi_res, &addr);
    if (!ACPI_SUCCESS(status))
        return AE_OK;

    if (addr.ResourceType == ACPI_MEMORY_RANGE)
    {
        root = &iomem_resource;
        flags = IORESOURCE_MEM;
        if (addr.Info.Mem.Caching == ACPI_PREFETCHABLE_MEMORY)
            flags |= IORESOURCE_PREFETCH;
    }
    else if (addr.ResourceType == ACPI_IO_RANGE)
    {
        root = &ioport_resource;
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

#if 0
    conflict = insert_resource_conflict(root, res);
    if (conflict) {
        dev_err(&info->bridge->dev,
            "address space collision: host bridge window %pR "
            "conflicts with %s %pR\n",
            res, conflict->name, conflict);
    } else {
        pci_bus_add_resource(info->bus, res, 0);
        info->res_num++;
        if (addr.translation_offset)
            dev_info(&info->bridge->dev, "host bridge window %pR "
                 "(PCI address [%#llx-%#llx])\n",
                 res, res->start - addr.translation_offset,
                 res->end - addr.translation_offset);
        else
            dev_info(&info->bridge->dev,
                 "host bridge window %pR\n", res);
    }
    return AE_OK;
#endif
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

    vsprintf(buf,"PCI Bus %04x:%02x", domain, busnum);
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
//            bus->subordinate = pci_scan_child_bus(bus);
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
//    result = acpi_pci_bind_root(device);
//    if (result)
//        goto end;

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
//    list_for_each_entry(child, &device->children, node)
//        acpi_pci_bridge_scan(child);

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


u32_t drvEntry(int action, char *cmdline)
{
    u32_t retval;

    ACPI_STATUS status;

    int i;

    if(action != 1)
        return 0;

    if( !dbg_open("/rd/1/drivers/acpi.log") )
    {
        printf("Can't open /rd/1/drivers/acpi.log\nExit\n");
        return 0;
    }

    status = AcpiReallocateRootTable();
    if (ACPI_FAILURE(status)) {
        dbgprintf("Unable to reallocate ACPI tables\n");
        goto err;
    }

    status = AcpiInitializeSubsystem();
    if (status != AE_OK) {
          dbgprintf("AcpiInitializeSubsystem failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    status = AcpiInitializeTables(NULL, 0, TRUE);
    if (status != AE_OK) {
          dbgprintf("AcpiInitializeTables failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    status = AcpiLoadTables();
    if (status != AE_OK) {
          dbgprintf("AcpiLoadTables failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

//    u32_t mode = ACPI_NO_HARDWARE_INIT | ACPI_NO_ACPI_ENABLE;

    status = AcpiEnableSubsystem(0);
    if (status != AE_OK) {
        dbgprintf("AcpiEnableSubsystem failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    status = AcpiInitializeObjects (0);
    if (ACPI_FAILURE (status))
    {
        dbgprintf("AcpiInitializeObjects failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }


    set_pic_mode(IO_APIC);

    acpi_scan();

//    print_device_tree(acpi_root);

    acpi_init_pci(acpi_root);

/*
    ACPI_HANDLE bus_handle;
    ACPI_HANDLE pci_root;

    status = AcpiGetHandle(0, "\\_SB_", &bus_handle);
    dbgprintf("system bus handle %x\n", bus_handle);

    status = AcpiGetHandle(bus_handle, "PCI0", &pci_root);
    if (status != AE_OK) {
          dbgprintf("AcpiGetHandle failed (%s)\n",
                     AcpiFormatException(status));
          goto err;
    }

    AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, 100,
                      get_device_by_hid_callback, NULL, NULL, NULL);
*/

#if 0

    AcpiWalkNamespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT, 4,
                      get_device_by_hid_callback, NULL, NULL, NULL);

    ACPI_OBJECT obj;
    ACPI_HANDLE bus_handle;
    ACPI_HANDLE pci_root;

    status = AcpiGetHandle(0, "\\_SB_", &bus_handle);
    dbgprintf("system bus handle %x\n", bus_handle);

    status = AcpiGetHandle(bus_handle, "PCI0", &pci_root);

    if (status != AE_OK) {
        dbgprintf("AcpiGetHandle failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    dbgprintf("pci root handle %x\n\n", pci_root);

    ACPI_BUFFER prt_buffer;

    prt_buffer.Length = ACPI_ALLOCATE_BUFFER;
    prt_buffer.Pointer = NULL;

    status = AcpiGetIrqRoutingTable(pci_root, &prt_buffer);

    if (status != AE_OK) {
        dbgprintf("AcpiGetIrqRoutingTable failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    prt_walk_table(&prt_buffer);


    ACPI_OBJECT arg = { ACPI_TYPE_INTEGER };
    ACPI_OBJECT_LIST arg_list = { 1, &arg };

    arg.Integer.Value = ACPI_IRQ_MODEL_IOAPIC;

    dbgprintf("\nset ioapic mode\n\n");

    status = AcpiEvaluateObject(NULL, "\\_PIC", &arg_list, NULL);

    if (ACPI_FAILURE(status)) {
        dbgprintf("AcpiEvaluateObject failed (%s)\n",
            AcpiFormatException(status));
 //       goto err;
    }


    status = AcpiGetIrqRoutingTable(pci_root, &prt_buffer);

    if (status != AE_OK) {
        dbgprintf("AcpiGetIrqRoutingTable failed (%s)\n",
            AcpiFormatException(status));
        goto err;
    }

    prt_walk_table(&prt_buffer);

    u8_t pin = PciRead8 (0, (31<<3) | 1, 0x3D);
    dbgprintf("bus 0 device 31 function 1 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 2, 0x3D);
    dbgprintf("bus 0 device 31 function 2 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 3, 0x3D);
    dbgprintf("bus 0 device 31 function 3 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 4, 0x3D);
    dbgprintf("bus 0 device 31 function 4 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 5, 0x3D);
    dbgprintf("bus 0 device 31 function 5 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 6, 0x3D);
    dbgprintf("bus 0 device 31 function 6 pin %d\n", pin-1);

    pin = PciRead8 (0, (31<<3) | 7, 0x3D);
    dbgprintf("bus 0 device 31 function 7 pin %d\n", pin-1);
#endif

err:

    return 0;

};

#if 0
    scan_devices();

    {
        bool retval = false;
        u32_t bus, last_bus;

        if( (last_bus = PciApi(1))==-1)
            return retval;

        dbgprintf("last bus %x\n", last_bus);

        for(bus=0; bus <= last_bus; bus++)
        {
            u32_t dev;

            for(dev = 0; dev < 32; dev++)
            {
                u32_t fn;

                for(fn = 0; fn < 8; fn++)
                {

                    u32_t id;
                    u32_t irq_bios, irq_acpi;
                    u32_t irq_pin;
                    u16_t pcicmd;
                    u32_t tmp;

                    u32_t devfn = (dev<<3 )|fn;

                    id = PciRead32(bus,devfn, PCI_VENDOR_ID);

    /* some broken boards return 0 or ~0 if a slot is empty: */
                if (id == 0xffffffff || id == 0x00000000 ||
                    id == 0x0000ffff || id == 0xffff0000)
                    continue;

                pcicmd = PciRead16(bus,devfn, PCI_COMMAND);
                if (! pcicmd & PCI_COMMAND_IO)
                    continue;

                tmp = PciRead32(bus,devfn, 0x3C);

                irq_bios = tmp & 0xFF;
                irq_pin  = (tmp >> 8) & 0xFF;

                int slot = (fn >> 3) & 0x1f;

                irq_acpi = irqtable[ dev * PCI_MAX_PINS +(irq_pin-1) ];

                if( irq_acpi < 0)
                    dbgprintf("PCI: no ACPI IRQ routing for "
                    "device %d.%d.%d INT%c\n",bus,dev,fn,'A'+irq_pin-1);

                dbgprintf("pci device %x_%x bus %d dev %d fn %d,"
                          "IRQ PIN %d BIOS IRQ %d ACPI IRQ %d\n",
                          id & 0xFFFF, id>>16, bus, dev, fn, irq_pin, irq_bios, irq_acpi);
                };
            }
        };
    };
#endif


#if 0

ACPI_STATUS
get_device_by_hid_callback(ACPI_HANDLE obj, u32_t depth, void* context,
    void** retval)
{
    static u32_t counter = 0;
    static char buff[256];

    ACPI_STATUS status;

    ACPI_BUFFER buffer;

    ACPI_DEVICE_INFO *info;

   // *retval = NULL;

    buffer.Length = 255;
    buffer.Pointer = buff;

    status = AcpiGetName(obj, ACPI_FULL_PATHNAME, &buffer);
    if (status != AE_OK) {
        return AE_CTRL_TERMINATE;
    }

    buff[buffer.Length] = '\0';

    dbgprintf("device %d %s ", counter, buff);

    status = AcpiGetObjectInfo(obj, &info);

    if (ACPI_SUCCESS (status))
    {
        if (info->Valid & ACPI_VALID_HID)
            dbgprintf (" HID: %s", info->HardwareId.String);

    };

    dbgprintf("\n");
    counter++;

    return AE_OK;
}

prt_walk_table(ACPI_BUFFER *prt)
{
    ACPI_PCI_ROUTING_TABLE *entry;
    char *prtptr;

    /* First check to see if there is a table to walk. */
    if (prt == NULL || prt->Pointer == NULL)
        return;

    /* Walk the table executing the handler function for each entry. */
    prtptr = prt->Pointer;
    entry = (ACPI_PCI_ROUTING_TABLE *)prtptr;
    while (entry->Length != 0)
    {

        dbgprintf("adress: %x %x  ", (u32_t)(entry->Address>>32),
                  (u32_t)entry->Address);
        dbgprintf("pin: %d  index: %d  source: %s\n",
                   entry->Pin,
                   entry->SourceIndex,
                   entry->Source);

//      handler(entry, arg);
        prtptr += entry->Length;
        entry = (ACPI_PCI_ROUTING_TABLE *)prtptr;
    }
}


static void add_irq(unsigned dev, unsigned pin, u8_t irq)
{
//    assert(dev < PCI_MAX_DEVICES && pin < PCI_MAX_PINS);

    irqtable[dev * PCI_MAX_PINS + pin] = irq;
}

static ACPI_STATUS get_irq_resource(ACPI_RESOURCE *res, void *context)
{
    ACPI_PCI_ROUTING_TABLE *tbl = (ACPI_PCI_ROUTING_TABLE *) context;

    if (res->Type == ACPI_RESOURCE_TYPE_IRQ)
    {
        ACPI_RESOURCE_IRQ *irq;

        irq = &res->Data.Irq;
        add_irq(tbl->Address >> 16, tbl->Pin,
                irq->Interrupts[tbl->SourceIndex]);
    } else if (res->Type == ACPI_RESOURCE_TYPE_EXTENDED_IRQ)
    {
        ACPI_RESOURCE_EXTENDED_IRQ *irq;

        add_irq(tbl->Address >> 16, tbl->Pin,
                irq->Interrupts[tbl->SourceIndex]);
    }

    return AE_OK;
}

char buff[4096];

static ACPI_STATUS get_pci_irq_routing(ACPI_HANDLE handle)
{
    ACPI_STATUS status;
    ACPI_BUFFER abuff;
    ACPI_PCI_ROUTING_TABLE *tbl;

    abuff.Length = sizeof(buff);
    abuff.Pointer = buff;

    status = AcpiGetIrqRoutingTable(handle, &abuff);
    if (ACPI_FAILURE(status)) {
        return AE_OK;
    }

    for (tbl = (ACPI_PCI_ROUTING_TABLE *)abuff.Pointer; tbl->Length;
            tbl = (ACPI_PCI_ROUTING_TABLE *)
            ((char *)tbl + tbl->Length))
    {
        ACPI_HANDLE src_handle;

        if (*(char*)tbl->Source == '\0') {
            add_irq(tbl->Address >> 16, tbl->Pin, tbl->SourceIndex);
            continue;
        }

        status = AcpiGetHandle(handle, tbl->Source, &src_handle);
        if (ACPI_FAILURE(status)) {
            printf("Failed AcpiGetHandle\n");
            continue;
        }
        status = AcpiWalkResources(src_handle, METHOD_NAME__CRS,
                get_irq_resource, tbl);
        if (ACPI_FAILURE(status)) {
            printf("Failed IRQ resource\n");
            continue;
        }
    }

    return AE_OK;
}

static ACPI_STATUS add_pci_root_dev(ACPI_HANDLE handle,
                UINT32 level,
                void *context,
                void **retval)
{
    int i;
    static unsigned called;

    if (++called > 1) {
        dbgprintf("ACPI: Warning! Multi rooted PCI is not supported!\n");
        return AE_OK;
    }

    for (i = 0; i < IRQ_TABLE_ENTRIES; i++)
        irqtable[i] = -1;

    return get_pci_irq_routing(handle);
}

static ACPI_STATUS add_pci_dev(ACPI_HANDLE handle,
                UINT32 level,
                void *context,
                void **retval)
{
    /* skip pci root when we get to it again */
    if (handle == pci_root_handle)
        return AE_OK;

    return get_pci_irq_routing(handle);
}

static void scan_devices(void)
{
    ACPI_STATUS status;

    /* get the root first */
    status = AcpiGetDevices("PNP0A03", add_pci_root_dev, NULL, NULL);
    if (status != AE_OK) {
        dbgprintf("scan_devices failed (%s)\n",
                   AcpiFormatException(status));
          return;
    }

//    assert(ACPI_SUCCESS(status));

    /* get the rest of the devices that implement _PRT */
    status = AcpiGetDevices(NULL, add_pci_dev, NULL, NULL);
//    assert(ACPI_SUCCESS(status));
}

#endif

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


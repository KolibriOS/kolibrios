
#include <types.h>
#include <syscall.h>

#include "acpi.h"

#define ACPI_NS_ROOT_PATH       "\\"
#define ACPI_NS_SYSTEM_BUS      "_SB_"

enum acpi_irq_model_id {
	ACPI_IRQ_MODEL_PIC = 0,
	ACPI_IRQ_MODEL_IOAPIC,
	ACPI_IRQ_MODEL_IOSAPIC,
	ACPI_IRQ_MODEL_PLATFORM,
	ACPI_IRQ_MODEL_COUNT
};


#define  addr_offset(addr, off) \
    (addr_t)((addr_t)(addr) + (addr_t)(off))

//#define acpi_remap( addr ) \
//    (addr_t)((addr_t)(addr) + OS_BASE)

#define acpi_remap( addr ) MapIoMem((void*)(addr),4096, 0x01)

ACPI_STATUS
get_device_by_hid_callback(ACPI_HANDLE obj, u32_t depth, void* context,
    void** retval)
{
    static u32_t counter = 0;
    static char buff[256];

	ACPI_STATUS status;

    ACPI_BUFFER buffer;

    ACPI_DEVICE_INFO info;

   // *retval = NULL;

    buffer.Length = 255;
    buffer.Pointer = buff;

    status = AcpiGetName(obj, ACPI_FULL_PATHNAME, &buffer);
    if (status != AE_OK) {
        return AE_CTRL_TERMINATE;
    }

    buff[buffer.Length] = '\0';

    dbgprintf("device %d %s ", counter, buff);

/*
    buffer.Pointer = &info;
    memset(&info, 0, sizeof(ACPI_DEVICE_INFO));
    status = AcpiGetObjectInfo(obj, &buffer.Pointer);

    if (ACPI_SUCCESS (status))
    {
        dbgprintf (" HID: %s, ADR: %x %x, Status: %x",
        info.HardwareId.String,
        (UINT32)(info.Address>>32),(UINT32)info.Address,
        info.CurrentStatus);
    };
*/

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




u32_t drvEntry(int action, char *cmdline)
{
    u32_t retval;

    int i;

    if(action != 1)
        return 0;

      ACPI_STATUS status;

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

    u32_t mode = ACPI_FULL_INITIALIZATION;

    status = AcpiEnableSubsystem(mode);
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

    AcpiWalkNamespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT, 4,
                      get_device_by_hid_callback, NULL, NULL, NULL);

#if 0

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


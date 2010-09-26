
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"

#define ACPI_BUS_CLASS          "system_bus"
#define ACPI_BUS_HID            "KLBSYBUS"
#define ACPI_BUS_DEVICE_NAME    "System Bus"


#define ACPI_IS_ROOT_DEVICE(device)    (!(device)->parent)

static LIST_HEAD(acpi_device_list);
static LIST_HEAD(acpi_bus_id_list);
DEFINE_MUTEX(acpi_device_lock);

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

enum acpi_bus_device_type {
    ACPI_BUS_TYPE_DEVICE = 0,
    ACPI_BUS_TYPE_POWER,
    ACPI_BUS_TYPE_PROCESSOR,
    ACPI_BUS_TYPE_THERMAL,
    ACPI_BUS_TYPE_POWER_BUTTON,
    ACPI_BUS_TYPE_SLEEP_BUTTON,
    ACPI_BUS_DEVICE_TYPE_COUNT
};

/*
 * _HID definitions
 * HIDs must conform to ACPI spec(6.1.4)
 * KolibriOS specific HIDs do not apply to this and begin with KOS:
 */

#define ACPI_POWER_HID              "KLBPOWER"
#define ACPI_PROCESSOR_OBJECT_HID   "KLBCPU"
#define ACPI_SYSTEM_HID             "KLBSYSTM"
#define ACPI_THERMAL_HID            "KLBTHERM"
#define ACPI_BUTTON_HID_POWERF      "KLBPWRBN"
#define ACPI_BUTTON_HID_SLEEPF      "KLBSLPBN"
#define ACPI_VIDEO_HID              "KLBVIDEO"
#define ACPI_BAY_HID                "KLBIOBAY"
#define ACPI_DOCK_HID               "KLBDOCK"
/* Quirk for broken IBM BIOSes */
#define ACPI_SMBUS_IBM_HID      "SMBUSIBM"


#define STRUCT_TO_INT(s)        (*((int*)&s))

#define ACPI_STA_DEFAULT (ACPI_STA_DEVICE_PRESENT | ACPI_STA_DEVICE_ENABLED | \
              ACPI_STA_DEVICE_UI      | ACPI_STA_DEVICE_FUNCTIONING)

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


struct acpi_bus_ops
{
    u32_t acpi_op_add:1;
    u32_t acpi_op_start:1;
};

struct acpi_device_flags {
    u32 dynamic_status:1;
    u32 bus_address:1;
    u32 removable:1;
    u32 ejectable:1;
    u32 lockable:1;
    u32 suprise_removal_ok:1;
    u32 power_manageable:1;
    u32 performance_manageable:1;
    u32 wake_capable:1; /* Wakeup(_PRW) supported? */
    u32 force_power_state:1;
    u32 reserved:22;
};

struct acpi_device_status {
    u32 present:1;
    u32 enabled:1;
    u32 show_in_ui:1;
    u32 functional:1;
    u32 battery_present:1;
    u32 reserved:27;
};


typedef char acpi_bus_id[8];
typedef unsigned long acpi_bus_address;
typedef char acpi_device_name[40];
typedef char acpi_device_class[20];

struct acpi_hardware_id {
    struct list_head list;
    char *id;
};

struct acpi_device_pnp
{
    acpi_bus_id       bus_id;       /* Object name */
    acpi_bus_address  bus_address;  /* _ADR */
    char *unique_id;                /* _UID */
    struct list_head  ids;          /* _HID and _CIDs */
    acpi_device_name  device_name;  /* Driver-determined */
    acpi_device_class device_class; /*        "          */
};


struct acpi_device
{
    int device_type;
    ACPI_HANDLE handle;     /* no handle for fixed hardware */
    struct acpi_device *parent;
    struct list_head children;
    struct list_head node;
//    struct list_head wakeup_list;
    struct acpi_device_status status;
    struct acpi_device_flags flags;
    struct acpi_device_pnp pnp;
//    struct acpi_device_power power;
//    struct acpi_device_wakeup wakeup;
//    struct acpi_device_perf performance;
//    struct acpi_device_dir dir;
//    struct acpi_device_ops ops;
//    struct acpi_driver *driver;
    void *driver_data;
//    struct device dev;
    struct acpi_bus_ops bus_ops;    /* workaround for different code path for hotplug */
 //   enum acpi_bus_removal_type removal_type;    /* indicate for different removal type */
};

struct acpi_device *acpi_root;


static void
acpi_util_eval_error(ACPI_HANDLE h, ACPI_STRING p, ACPI_STATUS s)
{
#ifdef ACPI_DEBUG_OUTPUT
    char prefix[80] = {'\0'};
    ACPI_BUFFER buffer = {sizeof(prefix), prefix};
    AcpiGetName(h, ACPI_FULL_PATHNAME, &buffer);
    ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Evaluate [%s.%s]: %s\n",
        (char *) prefix, p, AcpiFormatException(s)));
#else
    return;
#endif
}

ACPI_STATUS
acpi_evaluate_integer(ACPI_HANDLE handle, ACPI_STRING pathname,
              ACPI_OBJECT_LIST *arguments, unsigned long long *data)
{
    ACPI_STATUS status = AE_OK;
    ACPI_OBJECT element;
    ACPI_BUFFER buffer = { 0, NULL };

    if (!data)
        return AE_BAD_PARAMETER;

    buffer.Length = sizeof(ACPI_OBJECT);
    buffer.Pointer = &element;
    status = AcpiEvaluateObject(handle, pathname, arguments, &buffer);
    if (ACPI_FAILURE(status)) {
        acpi_util_eval_error(handle, pathname, status);
        return status;
    }

    if (element.Type != ACPI_TYPE_INTEGER) {
        acpi_util_eval_error(handle, pathname, AE_BAD_DATA);
        return AE_BAD_DATA;
    }

    *data = element.Integer.Value;

    ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Return value [%llu]\n", *data));

    return AE_OK;
}

void acpi_bus_data_handler(ACPI_HANDLE handle, void *context)
{

    /* TBD */

    return;
}


int acpi_bus_get_device(ACPI_HANDLE handle, struct acpi_device **device)
{
    ACPI_STATUS status = AE_OK;

    if (!device)
    {
        return -EINVAL;
    };

    /* TBD: Support fixed-feature devices */

    status = AcpiGetData(handle, acpi_bus_data_handler, (void **)device);
    if (ACPI_FAILURE(status) || !*device) {
        ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No context for object [%p]\n",
                  handle));
        return -ENODEV;
    }
    return 0;
}


ACPI_STATUS acpi_bus_get_status_handle(ACPI_HANDLE handle,
                       unsigned long long *sta)
{
    ACPI_STATUS status;

    status = acpi_evaluate_integer(handle, "_STA", NULL, sta);
    if (ACPI_SUCCESS(status))
    {
        return AE_OK;
    };

    if (status == AE_NOT_FOUND)
    {
        *sta = ACPI_STA_DEVICE_PRESENT | ACPI_STA_DEVICE_ENABLED |
               ACPI_STA_DEVICE_UI      | ACPI_STA_DEVICE_FUNCTIONING;
        return AE_OK;
    }
    return status;
}



static int acpi_bus_type_and_status(ACPI_HANDLE handle, int *type,
                    unsigned long long *sta)
{
    ACPI_STATUS status;
    ACPI_OBJECT_TYPE acpi_type;

    status = AcpiGetType(handle, &acpi_type);
    if (ACPI_FAILURE(status))
    {
        return -ENODEV;
    };

    switch (acpi_type)
    {
        case ACPI_TYPE_ANY:     /* for ACPI_ROOT_OBJECT */
        case ACPI_TYPE_DEVICE:
            *type = ACPI_BUS_TYPE_DEVICE;
            status = acpi_bus_get_status_handle(handle, sta);
            if (ACPI_FAILURE(status))
            {
                return -ENODEV;
            };
            break;

        case ACPI_TYPE_PROCESSOR:
            *type = ACPI_BUS_TYPE_PROCESSOR;
            status = acpi_bus_get_status_handle(handle, sta);
            if (ACPI_FAILURE(status))
            {
                return -ENODEV;
            };
            break;
        case ACPI_TYPE_THERMAL:
            *type = ACPI_BUS_TYPE_THERMAL;
            *sta = ACPI_STA_DEFAULT;
            break;
        case ACPI_TYPE_POWER:
            *type = ACPI_BUS_TYPE_POWER;
            *sta = ACPI_STA_DEFAULT;
            break;
        default:
            return -ENODEV;
    }

    return 0;
}

static struct acpi_device *acpi_bus_get_parent(ACPI_HANDLE handle)
{
    ACPI_STATUS status;
    struct      acpi_device *device;
    int         ret;

    /*
     * Fixed hardware devices do not appear in the namespace and do not
     * have handles, but we fabricate acpi_devices for them, so we have
     * to deal with them specially.
     */
    if (handle == NULL)
    {
        return acpi_root;
    };

    do
    {
        status = AcpiGetParent(handle, &handle);
        if (status == AE_NULL_ENTRY)
        {
            return NULL;
        };
        if (ACPI_FAILURE(status))
        {
            return acpi_root;
        };

        ret = acpi_bus_get_device(handle, &device);
        if (ret == 0)
        {
            return device;
        };
    } while (1);
}


static void acpi_device_get_busid(struct acpi_device *device)
{
    char bus_id[5] = { '?', 0 };
    struct acpi_buffer buffer = { sizeof(bus_id), bus_id };
    int i = 0;

    /*
     * Bus ID
     * ------
     * The device's Bus ID is simply the object name.
     * TBD: Shouldn't this value be unique (within the ACPI namespace)?
     */
    if (ACPI_IS_ROOT_DEVICE(device)) {
        strcpy(device->pnp.bus_id, "ACPI");
        return;
    }

    switch (device->device_type)
    {
        case ACPI_BUS_TYPE_POWER_BUTTON:
            strcpy(device->pnp.bus_id, "PWRF");
            break;
        case ACPI_BUS_TYPE_SLEEP_BUTTON:
            strcpy(device->pnp.bus_id, "SLPF");
            break;
        default:
            AcpiGetName(device->handle, ACPI_SINGLE_NAME, &buffer);
        /* Clean up trailing underscores (if any) */
            for (i = 3; i > 1; i--)
            {
                if (bus_id[i] == '_')
                    bus_id[i] = '\0';
                else
                    break;
            }
            strcpy(device->pnp.bus_id, bus_id);
            break;
    }
}


static int acpi_bus_get_flags(struct acpi_device *device)
{
    ACPI_STATUS status = AE_OK;
    ACPI_HANDLE temp   = NULL;

    /* Presence of _STA indicates 'dynamic_status' */
    status = AcpiGetHandle(device->handle, "_STA", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.dynamic_status = 1;

    /* Presence of _RMV indicates 'removable' */
    status = AcpiGetHandle(device->handle, "_RMV", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.removable = 1;

    /* Presence of _EJD|_EJ0 indicates 'ejectable' */
    status = AcpiGetHandle(device->handle, "_EJD", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.ejectable = 1;
    else {
        status = AcpiGetHandle(device->handle, "_EJ0", &temp);
        if (ACPI_SUCCESS(status))
            device->flags.ejectable = 1;
    }

    /* Presence of _LCK indicates 'lockable' */
    status = AcpiGetHandle(device->handle, "_LCK", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.lockable = 1;

    /* Presence of _PS0|_PR0 indicates 'power manageable' */
    status = AcpiGetHandle(device->handle, "_PS0", &temp);
    if (ACPI_FAILURE(status))
        status = AcpiGetHandle(device->handle, "_PR0", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.power_manageable = 1;

    /* Presence of _PRW indicates wake capable */
    status = AcpiGetHandle(device->handle, "_PRW", &temp);
    if (ACPI_SUCCESS(status))
        device->flags.wake_capable = 1;

    /* TBD: Performance management */

    return 0;
}

/*
 * acpi_bay_match - see if a device is an ejectable driver bay
 *
 * If an acpi object is ejectable and has one of the ACPI ATA methods defined,
 * then we can safely call it an ejectable drive bay
 */
static int acpi_bay_match(struct acpi_device *device){
    ACPI_STATUS status;
    ACPI_HANDLE handle;
    ACPI_HANDLE tmp;
    ACPI_HANDLE phandle;

    handle = device->handle;

    status = AcpiGetHandle(handle, "_EJ0", &tmp);
    if (ACPI_FAILURE(status))
        return -ENODEV;

    if ((ACPI_SUCCESS(AcpiGetHandle(handle, "_GTF", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(handle, "_GTM", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(handle, "_STM", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(handle, "_SDD", &tmp))))
        return 0;

    if (AcpiGetParent(handle, &phandle))
        return -ENODEV;

    if ((ACPI_SUCCESS(AcpiGetHandle(phandle, "_GTF", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(phandle, "_GTM", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(phandle, "_STM", &tmp))) ||
        (ACPI_SUCCESS(AcpiGetHandle(phandle, "_SDD", &tmp))))
        return 0;

    return -ENODEV;
}

/*
 * acpi_dock_match - see if a device has a _DCK method
 */
static int acpi_dock_match(struct acpi_device *device)
{
    ACPI_HANDLE tmp;
    return AcpiGetHandle(device->handle, "_DCK", &tmp);
}

char *acpi_device_hid(struct acpi_device *device)
{
    struct acpi_hardware_id *hid;

    hid = list_first_entry(&device->pnp.ids, struct acpi_hardware_id, list);
    return hid->id;
}



static void acpi_add_id(struct acpi_device *device, const char *dev_id)
{
    struct acpi_hardware_id *id;

    id = kmalloc(sizeof(*id), GFP_KERNEL);
    if (!id)
    {
        return;
    };

    INIT_LIST_HEAD(&id->list);

    id->id = kmalloc(strlen(dev_id) + 1, GFP_KERNEL);
    if (!id->id) {
        kfree(id);
        return;
    }

    strcpy(id->id, dev_id);

    list_add_tail(&id->list, &device->pnp.ids);
}

#define ACPI_VIDEO_OUTPUT_SWITCHING         0x0001
#define ACPI_VIDEO_DEVICE_POSTING           0x0002
#define ACPI_VIDEO_ROM_AVAILABLE            0x0004
#define ACPI_VIDEO_BACKLIGHT                0x0008
#define ACPI_VIDEO_BACKLIGHT_FORCE_VENDOR       0x0010
#define ACPI_VIDEO_BACKLIGHT_FORCE_VIDEO        0x0020
#define ACPI_VIDEO_OUTPUT_SWITCHING_FORCE_VENDOR    0x0040
#define ACPI_VIDEO_OUTPUT_SWITCHING_FORCE_VIDEO     0x0080
#define ACPI_VIDEO_BACKLIGHT_DMI_VENDOR         0x0100
#define ACPI_VIDEO_BACKLIGHT_DMI_VIDEO          0x0200
#define ACPI_VIDEO_OUTPUT_SWITCHING_DMI_VENDOR      0x0400
#define ACPI_VIDEO_OUTPUT_SWITCHING_DMI_VIDEO       0x0800


long acpi_is_video_device(struct acpi_device *device)
{
    ACPI_HANDLE h_dummy;
    long video_caps = 0;

    if (!device)
        return 0;

    /* Is this device able to support video switching ? */
    if (ACPI_SUCCESS(AcpiGetHandle(device->handle, "_DOD", &h_dummy)) ||
        ACPI_SUCCESS(AcpiGetHandle(device->handle, "_DOS", &h_dummy)))
        video_caps |= ACPI_VIDEO_OUTPUT_SWITCHING;

    /* Is this device able to retrieve a video ROM ? */
    if (ACPI_SUCCESS(AcpiGetHandle(device->handle, "_ROM", &h_dummy)))
        video_caps |= ACPI_VIDEO_ROM_AVAILABLE;

    /* Is this device able to configure which video head to be POSTed ? */
    if (ACPI_SUCCESS(AcpiGetHandle(device->handle, "_VPO", &h_dummy)) &&
        ACPI_SUCCESS(AcpiGetHandle(device->handle, "_GPD", &h_dummy)) &&
        ACPI_SUCCESS(AcpiGetHandle(device->handle, "_SPD", &h_dummy)))
        video_caps |= ACPI_VIDEO_DEVICE_POSTING;

     return video_caps;
}


static void acpi_device_set_id(struct acpi_device *device)
{
    ACPI_STATUS status;
    ACPI_DEVICE_INFO *info;
    ACPI_DEVICE_ID_LIST *cid_list;
    int i;

    switch (device->device_type)
    {
        case ACPI_BUS_TYPE_DEVICE:
            if (ACPI_IS_ROOT_DEVICE(device))
            {
                acpi_add_id(device, ACPI_SYSTEM_HID);
                break;
            }

            status = AcpiGetObjectInfo(device->handle, &info);
            if (ACPI_FAILURE(status)) {
                printk(KERN_ERR "%s: Error reading device info\n", __func__);
                return;
            }

            if (info->Valid & ACPI_VALID_HID)
                acpi_add_id(device, info->HardwareId.String);
            if (info->Valid & ACPI_VALID_CID)
            {
                cid_list = &info->CompatibleIdList;
                for (i = 0; i < cid_list->Count; i++)
                    acpi_add_id(device, cid_list->Ids[i].String);
            }
            if (info->Valid & ACPI_VALID_ADR) {
                device->pnp.bus_address = info->Address;
                device->flags.bus_address = 1;
            }

            kfree(info);

        /*
         * Some devices don't reliably have _HIDs & _CIDs, so add
         * synthetic HIDs to make sure drivers can find them.
         */
        if (acpi_is_video_device(device))
            acpi_add_id(device, ACPI_VIDEO_HID);
        else if (ACPI_SUCCESS(acpi_bay_match(device)))
            acpi_add_id(device, ACPI_BAY_HID);
        else if (ACPI_SUCCESS(acpi_dock_match(device)))
            acpi_add_id(device, ACPI_DOCK_HID);
        else if (!acpi_device_hid(device) &&
             ACPI_IS_ROOT_DEVICE(device->parent)) {
            acpi_add_id(device, ACPI_BUS_HID); /* \_SB, LNXSYBUS */
            strcpy(device->pnp.device_name, ACPI_BUS_DEVICE_NAME);
            strcpy(device->pnp.device_class, ACPI_BUS_CLASS);
        }

        break;
    case ACPI_BUS_TYPE_POWER:
        acpi_add_id(device, ACPI_POWER_HID);
        break;
    case ACPI_BUS_TYPE_PROCESSOR:
        acpi_add_id(device, ACPI_PROCESSOR_OBJECT_HID);
        break;
    case ACPI_BUS_TYPE_THERMAL:
        acpi_add_id(device, ACPI_THERMAL_HID);
        break;
    case ACPI_BUS_TYPE_POWER_BUTTON:
        acpi_add_id(device, ACPI_BUTTON_HID_POWERF);
        break;
    case ACPI_BUS_TYPE_SLEEP_BUTTON:
        acpi_add_id(device, ACPI_BUTTON_HID_SLEEPF);
        break;
    }

    /*
     * We build acpi_devices for some objects that don't have _HID or _CID,
     * e.g., PCI bridges and slots.  Drivers can't bind to these objects,
     * but we do use them indirectly by traversing the acpi_device tree.
     * This generic ID isn't useful for driver binding, but it provides
     * the useful property that "every acpi_device has an ID."
     */
    if (list_empty(&device->pnp.ids))
        acpi_add_id(device, "device");
}


static int acpi_device_set_context(struct acpi_device *device)
{
    ACPI_STATUS status;

    /*
     * Context
     * -------
     * Attach this 'struct acpi_device' to the ACPI object.  This makes
     * resolutions from handle->device very efficient.  Fixed hardware
     * devices have no handles, so we skip them.
     */
    if (!device->handle)
        return 0;

    status = AcpiAttachData(device->handle,
                  acpi_bus_data_handler, device);
    if (ACPI_SUCCESS(status))
        return 0;

    dbgprintf(KERN_ERR "Error attaching device data\n");
    return -ENODEV;
}


static int acpi_device_register(struct acpi_device *device)
{
    int result;
    struct acpi_device_bus_id *acpi_device_bus_id, *new_bus_id;
    int found = 0;

    /*
     * Linkage
     * -------
     * Link this device to its parent and siblings.
     */
    INIT_LIST_HEAD(&device->children);
    INIT_LIST_HEAD(&device->node);

    new_bus_id = kzalloc(sizeof(struct acpi_device_bus_id), GFP_KERNEL);
    if (!new_bus_id) {
        dbgprintf(KERN_ERR "Memory allocation error\n");
        return -ENOMEM;
    }

    mutex_lock(&acpi_device_lock);
    /*
     * Find suitable bus_id and instance number in acpi_bus_id_list
     * If failed, create one and link it into acpi_bus_id_list
     */
    list_for_each_entry(acpi_device_bus_id, &acpi_bus_id_list, node)
    {
        if (!strcmp(acpi_device_bus_id->bus_id, acpi_device_hid(device)))
        {
            acpi_device_bus_id->instance_no++;
            found = 1;
            kfree(new_bus_id);
            break;
        }
    }
    if (!found)
    {
        acpi_device_bus_id = new_bus_id;
        strcpy(acpi_device_bus_id->bus_id, acpi_device_hid(device));
        acpi_device_bus_id->instance_no = 0;
        list_add_tail(&acpi_device_bus_id->node, &acpi_bus_id_list);
    }

//    dev_set_name(&device->dev, "%s:%02x", acpi_device_bus_id->bus_id, acpi_device_bus_id->instance_no);

    if (device->parent)
        list_add_tail(&device->node, &device->parent->children);

    mutex_unlock(&acpi_device_lock);

//    device->dev.bus = &acpi_bus_type;
//    device->dev.release = &acpi_device_release;
//    result = device_register(&device->dev);
//    if (result) {
//        dev_err(&device->dev, "Error registering device\n");
//        goto end;
//    }


//    device->removal_type = ACPI_BUS_REMOVAL_NORMAL;
    return 0;
end:
    mutex_lock(&acpi_device_lock);
    if (device->parent)
        list_del(&device->node);
    mutex_unlock(&acpi_device_lock);
    return result;
}



static int acpi_add_single_object(struct acpi_device **child,
                  ACPI_HANDLE handle, int type,
                  unsigned long long sta,
                  struct acpi_bus_ops *ops)
{
    int result;
    struct acpi_device *device;
    ACPI_BUFFER buffer = { ACPI_ALLOCATE_BUFFER, NULL };

    device = kzalloc(sizeof(struct acpi_device), GFP_KERNEL);
    if (!device) {
        dbgprintf("%s: Memory allocation error\n", __FUNCTION__);
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&device->pnp.ids);
    device->device_type = type;
    device->handle = handle;
    device->parent = acpi_bus_get_parent(handle);
    device->bus_ops = *ops; /* workround for not call .start */
    STRUCT_TO_INT(device->status) = sta;

    acpi_device_get_busid(device);

    /*
     * Flags
     * -----
     * Note that we only look for object handles -- cannot evaluate objects
     * until we know the device is present and properly initialized.
     */
    result = acpi_bus_get_flags(device);
    if (result)
        goto end;

    /*
     * Initialize Device
     * -----------------
     * TBD: Synch with Core's enumeration/initialization process.
     */
    acpi_device_set_id(device);


    if ((result = acpi_device_set_context(device)))
        goto end;

    result = acpi_device_register(device);

    /*
     * Bind _ADR-Based Devices when hot add
     */
//    if (device->flags.bus_address) {
//        if (device->parent && device->parent->ops.bind)
//            device->parent->ops.bind(device);
//    }

end:
    if (!result) {
        AcpiGetName(handle, ACPI_FULL_PATHNAME, &buffer);
        dbgprintf("Adding [%s]\n", (char *)buffer.Pointer);
        kfree(buffer.Pointer);
        *child = device;
    };
    return result;
}




static ACPI_STATUS acpi_bus_check_add(ACPI_HANDLE handle, u32 lvl,
                      void *context, void **return_value)
{
    struct acpi_bus_ops *ops = context;
    int type;
    unsigned long long sta;
    struct acpi_device *device;
    ACPI_STATUS status;
    int result;

    result = acpi_bus_type_and_status(handle, &type, &sta);

    if (result)
    {
        return AE_OK;
    };

    if (!(sta & ACPI_STA_DEVICE_PRESENT) &&
        !(sta & ACPI_STA_DEVICE_FUNCTIONING))
    {
        return AE_CTRL_DEPTH;
    };

    /*
     * We may already have an acpi_device from a previous enumeration.  If
     * so, we needn't add it again, but we may still have to start it.
     */
    device = NULL;
    acpi_bus_get_device(handle, &device);
    if (ops->acpi_op_add && !device)
        acpi_add_single_object(&device, handle, type, sta, ops);

    if (!device)
    {
        return AE_CTRL_DEPTH;
    };
/*
    if (ops->acpi_op_start && !(ops->acpi_op_add)) {
        status = acpi_start_single_object(device);
        if (ACPI_FAILURE(status))
            return AE_CTRL_DEPTH;
    }
*/

    if (!*return_value)
        *return_value = device;

    return AE_OK;
}



static int acpi_bus_scan(ACPI_HANDLE handle, struct acpi_bus_ops *ops,
             struct acpi_device **child)
{
    ACPI_STATUS status;
    void *device = NULL;

    ENTER();

    status = acpi_bus_check_add(handle, 0, ops, &device);

    if (ACPI_SUCCESS(status))
        AcpiWalkNamespace(ACPI_TYPE_ANY, handle, ACPI_UINT32_MAX,
                    acpi_bus_check_add, NULL, ops, &device);

    if (child)
        *child = device;

    LEAVE();

    if (device)
        return 0;
    else
        return -ENODEV;
}



int acpi_scan()
{
    int err;
    struct acpi_bus_ops ops;

    memset(&ops, 0, sizeof(ops));
    ops.acpi_op_add = 1;
    ops.acpi_op_start = 1;

    err = acpi_bus_scan(ACPI_ROOT_OBJECT, &ops, &acpi_root);

    return err;
};


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
        dbgprintf("ACPI: machine set to %s mode\n", mode ? "APIC" : "PIC");
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

    acpi_scan();

    print_device_tree(acpi_root);

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

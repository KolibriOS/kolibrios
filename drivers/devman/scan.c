

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

#define STRUCT_TO_INT(s)        (*((int*)&s))


extern struct acpi_device *acpi_root;

static LIST_HEAD(acpi_device_list);
static LIST_HEAD(acpi_bus_id_list);
DEFINE_MUTEX(acpi_device_lock);


struct acpi_device_bus_id{
	char bus_id[15];
	unsigned int instance_no;
	struct list_head node;
};


struct acpi_hardware_id {
    struct list_head list;
    char *id;
};

#define acpi_device_name(d) ((d)->pnp.device_name)
#define acpi_device_class(d)    ((d)->pnp.device_class)


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



/* --------------------------------------------------------------------------
			ACPI Bus operations
   -------------------------------------------------------------------------- */

int acpi_match_device_ids(struct acpi_device *device,
              const struct acpi_device_ids *ids)
{
    const struct acpi_device_ids *id;
    struct acpi_hardware_id *hwid;

    /*
     * If the device is not present, it is unnecessary to load device
     * driver for it.
     */
//    if (!device->status.present)
//        return -ENODEV;

    for (id = ids; id->id[0]; id++)
        list_for_each_entry(hwid, &device->pnp.ids, list)
            if (!strcmp((char *) id->id, hwid->id))
                return 0;

    return -ENOENT;
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
		printk(KERN_ERR PREFIX "Memory allocation error\n");
        return -ENOMEM;
    }

    mutex_lock(&acpi_device_lock);
    /*
     * Find suitable bus_id and instance number in acpi_bus_id_list
     * If failed, create one and link it into acpi_bus_id_list
     */
    list_for_each_entry(acpi_device_bus_id, &acpi_bus_id_list, node)
    {
		if (!strcmp(acpi_device_bus_id->bus_id,
                acpi_device_hid(device)))
        {
            acpi_device_bus_id->instance_no++;
            found = 1;
            kfree(new_bus_id);
            break;
        }
    };

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
        return acpi_root;

	do {
        status = AcpiGetParent(handle, &handle);
        if (status == AE_NULL_ENTRY)
            return NULL;
        if (ACPI_FAILURE(status))
            return acpi_root;

        ret = acpi_bus_get_device(handle, &device);
        if (ret == 0)
            return device;
    } while (1);
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
		for (i = 3; i > 1; i--) {
                if (bus_id[i] == '_')
                    bus_id[i] = '\0';
                else
                    break;
            }
            strcpy(device->pnp.bus_id, bus_id);
            break;
    }
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
        return;

    INIT_LIST_HEAD(&id->list);

    id->id = kmalloc(strlen(dev_id) + 1, GFP_KERNEL);
    if (!id->id) {
        kfree(id);
        return;
    }

    strcpy(id->id, dev_id);
    list_add_tail(&id->list, &device->pnp.ids);
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
		if (ACPI_IS_ROOT_DEVICE(device)) {
                acpi_add_id(device, ACPI_SYSTEM_HID);
                break;
            }

            status = AcpiGetObjectInfo(device->handle, &info);
            if (ACPI_FAILURE(status)) {
                printk(KERN_ERR PREFIX "%s: Error reading device info\n", __func__);
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

	printk(KERN_ERR PREFIX "Error attaching device data\n");
    return -ENODEV;
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
		printk(KERN_ERR PREFIX "Memory allocation error\n");
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
    if (device->flags.bus_address) {
        if (device->parent && device->parent->ops.bind)
            device->parent->ops.bind(device);
    }

end:
    if (!result) {
        AcpiGetName(handle, ACPI_FULL_PATHNAME, &buffer);
        dbgprintf(PREFIX "Adding [%s]\n", (char *)buffer.Pointer);
        kfree(buffer.Pointer);
        *child = device;
    };
    return result;
}

#define ACPI_STA_DEFAULT (ACPI_STA_DEVICE_PRESENT | ACPI_STA_DEVICE_ENABLED | \
			  ACPI_STA_DEVICE_UI      | ACPI_STA_DEVICE_FUNCTIONING)

static int acpi_bus_type_and_status(ACPI_HANDLE handle, int *type,
                    unsigned long long *sta)
{
    ACPI_STATUS status;
    ACPI_OBJECT_TYPE acpi_type;

    status = AcpiGetType(handle, &acpi_type);
    if (ACPI_FAILURE(status))
        return -ENODEV;

    switch (acpi_type)
    {
        case ACPI_TYPE_ANY:     /* for ACPI_ROOT_OBJECT */
        case ACPI_TYPE_DEVICE:
            *type = ACPI_BUS_TYPE_DEVICE;
            status = acpi_bus_get_status_handle(handle, sta);
            if (ACPI_FAILURE(status))
                return -ENODEV;
            break;
        case ACPI_TYPE_PROCESSOR:
            *type = ACPI_BUS_TYPE_PROCESSOR;
            status = acpi_bus_get_status_handle(handle, sta);
            if (ACPI_FAILURE(status))
                return -ENODEV;
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
        return AE_OK;

    if (!(sta & ACPI_STA_DEVICE_PRESENT) &&
        !(sta & ACPI_STA_DEVICE_FUNCTIONING))
        return AE_CTRL_DEPTH;

    /*
     * We may already have an acpi_device from a previous enumeration.  If
     * so, we needn't add it again, but we may still have to start it.
     */
    device = NULL;
    acpi_bus_get_device(handle, &device);
    if (ops->acpi_op_add && !device)
        acpi_add_single_object(&device, handle, type, sta, ops);

    if (!device)
        return AE_CTRL_DEPTH;

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

    status = acpi_bus_check_add(handle, 0, ops, &device);
    if (ACPI_SUCCESS(status))
        AcpiWalkNamespace(ACPI_TYPE_ANY, handle, ACPI_UINT32_MAX,
                    acpi_bus_check_add, NULL, ops, &device);

    if (child)
        *child = device;

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


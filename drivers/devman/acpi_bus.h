
struct acpi_device;


/*
 * ACPI Driver
 * -----------
 */

typedef int (*acpi_op_add) (struct acpi_device * device);
typedef int (*acpi_op_remove) (struct acpi_device * device, int type);
typedef int (*acpi_op_start) (struct acpi_device * device);
//typedef int (*acpi_op_suspend) (struct acpi_device * device,
//                pm_message_t state);
typedef int (*acpi_op_resume) (struct acpi_device * device);
typedef int (*acpi_op_bind) (struct acpi_device * device);
typedef int (*acpi_op_unbind) (struct acpi_device * device);
typedef void (*acpi_op_notify) (struct acpi_device * device, u32 event);

struct acpi_bus_ops {
    u32 acpi_op_add:1;
    u32 acpi_op_start:1;
};

struct acpi_device_ops {
    acpi_op_add add;
    acpi_op_remove remove;
    acpi_op_start start;
//    acpi_op_suspend suspend;
    acpi_op_resume resume;
    acpi_op_bind bind;
    acpi_op_unbind unbind;
    acpi_op_notify notify;
};

struct resource_list {
    struct resource_list *next;
    struct resource *res;
//    struct pci_dev *dev;
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



#define ACPI_ID_LEN     16 /* only 9 bytes needed here, 16 bytes are used */
                           /* to workaround crosscompile issues */

struct acpi_device_ids
{
    u8  id[ACPI_ID_LEN];
    u32 driver_data;
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
    struct acpi_device_ops ops;
//    struct acpi_driver *driver;
    void *driver_data;
//    struct device dev;
    struct acpi_bus_ops bus_ops;    /* workaround for different code path for hotplug */
 //   enum acpi_bus_removal_type removal_type;    /* indicate for different removal type */
};


struct acpi_pci_root {
    struct list_head node;
    struct acpi_device * device;
    struct acpi_pci_id id;
    struct pci_bus *bus;
    u16 segment;
    struct resource secondary;      /* downstream bus range */

};


#define acpi_device_bid(d)  ((d)->pnp.bus_id)
#define acpi_device_adr(d)  ((d)->pnp.bus_address)
char *acpi_device_hid(struct acpi_device *device);
#define acpi_device_name(d) ((d)->pnp.device_name)
#define acpi_device_class(d)    ((d)->pnp.device_class)

int acpi_match_device_ids(struct acpi_device *device,
              const struct acpi_device_ids *ids);

int acpi_pci_irq_add_prt(ACPI_HANDLE handle, struct pci_bus *bus);
int acpi_pci_bind_root(struct acpi_device *device);
struct pci_dev *acpi_get_pci_dev(ACPI_HANDLE handle);
int acpi_is_root_bridge(ACPI_HANDLE handle);

